/* Copyright (c) 2014, The Nuria Project
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *    1. The origin of this software must not be misrepresented; you must not
 *       claim that you wrote the original software. If you use this software
 *       in a product, an acknowledgment in the product documentation would be
 *       appreciated but is not required.
 *    2. Altered source versions must be plainly marked as such, and must not be
 *       misrepresented as being the original software.
 *    3. This notice may not be removed or altered from any source
 *       distribution.
 */

#include "private/variableaccessor.hpp"

#include <nuria/runtimemetaobject.hpp>
#include <nuria/debug.hpp>
#include <QtTest/QTest>

using namespace Nuria::Template;

// 
class TestQObject : public QObject {
	Q_OBJECT
	Q_PROPERTY(int v READ readValue)
	Q_PROPERTY(QVariantMap map READ readMap)
public:
	
	int readValue () { return value; }
	QVariantMap readMap () { return data; }
	
	int value = 1;
	QVariantMap data;
};

struct Struct {
	int integer = 1;
	int something () { return 123; }
};
 
class VariableAccessorTest : public QObject {
	Q_OBJECT
private slots:
	
	void initTestCase ();
	
	void run_data ();
	void run ();
	
	void walkHierarchy ();
	void findMethodOfMetaObject ();
	
private:
	Nuria::RuntimeMetaObject *metaObj = nullptr;
	TestQObject *testObject = nullptr;
};

Q_DECLARE_METATYPE(TestQObject*)
Q_DECLARE_METATYPE(Struct)

void VariableAccessorTest::initTestCase () {
	qRegisterMetaType< TestQObject * > ();
	qRegisterMetaType< Struct > ();
	
	// 
	this->testObject = new TestQObject;
	this->testObject->setParent (this);
	
	// 
	this->metaObj = new Nuria::RuntimeMetaObject ("Struct");
	this->metaObj->addField ("integer", "int", { }, [](void *s){ return QVariant (((Struct *)s)->integer); });
	this->metaObj->addMethod (Nuria::MetaMethod::Method, "something", "int", { }, { }, { },
	                          [] (void *s, Nuria::RuntimeMetaObject::InvokeAction) {
		return Nuria::Callback ((Struct *)s, &Struct::something);
	});
	
	this->metaObj->finalize ();
	Nuria::MetaObject::registerMetaObject (this->metaObj);
	
}

void VariableAccessorTest::run_data () {
	QTest::addColumn< QVariant > ("data");
	QTest::addColumn< QVariantList > ("chain");
	QTest::addColumn< bool > ("success");
	QTest::addColumn< QVariant > ("expected");
	
	QVariant list = QVariant (QVariantList { "a", "b", "c" });
	QVariant intList = QVariant::fromValue (QVector< int > { 1, 2, 3 });
	
	QVariant map = QVariant (QVariantMap { { "a", "A" }, { "b", "B" }, { "c", "C" } });
	QVariant stringMap = QVariant::fromValue (QMap< QString, QString > {
	                                                  { "a", "A" }, { "b", "B" }, { "c", "C" } });
	QVariant object = QVariant::fromValue (this->testObject);
	QVariant structure = QVariant::fromValue (Struct ());
	
	// 
	QTest::newRow ("not iterable") << QVariant ("abc") << QVariantList { 1 } << false << QVariant ();
	
	QTest::newRow ("list") << list << QVariantList { 1 } << true << QVariant ("b");
	QTest::newRow ("list out of bounds") << list << QVariantList { -1 } << false << QVariant ();
	
	QTest::newRow ("sequential") << intList << QVariantList { 1 } << true << QVariant (2);
	QTest::newRow ("sequential out of bounds") << intList << QVariantList { -1 } << false << QVariant ();
	
	QTest::newRow ("map") << map << QVariantList { "c" } << true << QVariant ("C");
	QTest::newRow ("map not found") << map << QVariantList { "foo" } << false << QVariant ();
	QTest::newRow ("map empty key") << map << QVariantList { "" } << false << QVariant ();
	
	QTest::newRow ("associative") << stringMap << QVariantList { "c" } << true << QVariant ("C");
	QTest::newRow ("associative not found") << stringMap << QVariantList { "foo" } << false << QVariant ();
	QTest::newRow ("associative empty key") << stringMap << QVariantList { "" } << false << QVariant ();
	
	QTest::newRow ("qobject property") << object << QVariantList { "v" } << true << QVariant (1);
	QTest::newRow ("qobject not found") << object << QVariantList { "foo" } << false << QVariant ();
	
	QTest::newRow ("metaobject field") << structure << QVariantList { "integer" } << true << QVariant (1);
	QTest::newRow ("metaobject not found") << structure << QVariantList { "foo" } << false << QVariant ();
	
}

void VariableAccessorTest::run () {
	QFETCH(QVariant, data);
	QFETCH(QVariantList, chain);
	QFETCH(bool, success);
	QFETCH(QVariant, expected);
	
	QCOMPARE(VariableAcessor::walkChain (data, chain, 0), success);
	
	if (success) {
		QCOMPARE(data, expected);
	}
	
}

void VariableAccessorTest::walkHierarchy () {
	this->testObject->data = { { "foo", QVariantList { QVariant::fromValue (Struct ()) } } };
	QVariant object = QVariant::fromValue (this->testObject);
	QVariant v = object;
	
	// 
	QVERIFY(VariableAcessor::walkChain (v, { "map", "foo", 0, "integer" }, 0));
	QCOMPARE(v, QVariant (1));
}

void VariableAccessorTest::findMethodOfMetaObject () {
	QVariant structure = QVariant::fromValue (Struct ());
	QVariant v = structure;
	
	QVERIFY(VariableAcessor::walkChain (v, { "something" }, 0));
	QCOMPARE(v.userType (), qMetaTypeId< Nuria::Callback > ());
	
	Nuria::Callback cb = v.value< Nuria::Callback > ();
	QVERIFY(cb.isValid ());
	QCOMPARE(cb (), QVariant (123));
	
}

QTEST_MAIN(VariableAccessorTest)
#include "tst_variableaccessor.moc"
