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
