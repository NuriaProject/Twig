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

#include "templateloader.hpp"
#include <nuria/debug.hpp>
#include <QtTest/QTest>

using namespace Nuria;

// 
class TemplateLoaderTest : public QObject {
	Q_OBJECT
private slots:
	
	void verifyHasTemplate ();
	void verifyLoad ();
	
};

void TemplateLoaderTest::verifyHasTemplate () {
	TemplateLoader loader;
	
	QVERIFY(!loader.hasTemplate (""));
	QVERIFY(loader.hasTemplate ("foo"));
	
}

void TemplateLoaderTest::verifyLoad () {
	TemplateLoader loader;
	
	QCOMPARE(loader.load ("foo"), QByteArray ("foo"));
	
}

QTEST_MAIN(TemplateLoaderTest)
#include "tst_templateloader.moc"
