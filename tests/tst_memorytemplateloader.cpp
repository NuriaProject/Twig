/* Copyright (c) 2014-2015, The Nuria Project
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * The NuriaProject Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with The NuriaProject Framework.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "nuria/memorytemplateloader.hpp"
#include <nuria/debug.hpp>
#include <QtTest/QTest>

using namespace Nuria;

// 
class MemoryTemplateLoaderTest : public QObject {
	Q_OBJECT
private slots:
	
	void verifyHasTemplate ();
	void verifyLoad ();
	void testAddingATemplate ();
	void replaceTemplateData ();
	void verifyRemovingATemplate ();
	void verifyResettingAllTemplates ();
	
private:
	
	MemoryTemplateLoader::Map testData = {
	        { "first", "one" },
	        { "second", "two" }
	};
	
};

void MemoryTemplateLoaderTest::verifyHasTemplate () {
	MemoryTemplateLoader loader (testData);
	
	QVERIFY(loader.hasTemplate ("first"));
	QVERIFY(loader.hasTemplate ("second"));
	QVERIFY(!loader.hasTemplate ("third"));
	
}

void MemoryTemplateLoaderTest::verifyLoad () {
	MemoryTemplateLoader loader (testData);
	
	QCOMPARE(loader.load ("first"), QByteArray ("one"));
	QCOMPARE(loader.load ("second"), QByteArray ("two"));
	QCOMPARE(loader.load ("third"), QByteArray ());
	
}

void MemoryTemplateLoaderTest::testAddingATemplate () {
	MemoryTemplateLoader loader (testData);
	auto expected = testData;
	expected.insert ("third", "three");
	
	QVERIFY(!loader.hasTemplate ("third"));
	loader.addTemplate ("third", "three");
	
	QVERIFY(loader.hasTemplate ("third"));
	QCOMPARE(loader.map (), expected);
	
}

void MemoryTemplateLoaderTest::replaceTemplateData () {
	MemoryTemplateLoader loader (testData);
	auto expected = testData;
	expected.insert ("first", "three");
	
	QVERIFY(loader.hasTemplate ("first"));
	loader.addTemplate ("first", "three");
	
	QVERIFY(loader.hasTemplate ("first"));
	QCOMPARE(loader.map (), expected);
	
}

void MemoryTemplateLoaderTest::verifyRemovingATemplate () {
	MemoryTemplateLoader loader (testData);
	auto expected = testData;
	expected.remove ("first");
	
	QVERIFY(loader.hasTemplate ("first"));
	loader.removeTemplate ("first");
	
	QVERIFY(!loader.hasTemplate ("first"));
	QCOMPARE(loader.map (), expected);
	
}

void MemoryTemplateLoaderTest::verifyResettingAllTemplates () {
	MemoryTemplateLoader loader (testData);
	MemoryTemplateLoader::Map data = { { "foo", "bar" }, { "nuria", "project" } };
	
	QCOMPARE(loader.map (), testData);
	loader.setMap (data);
	QCOMPARE(loader.map (), data);
	
}

QTEST_MAIN(MemoryTemplateLoaderTest)
#include "tst_memorytemplateloader.moc"
