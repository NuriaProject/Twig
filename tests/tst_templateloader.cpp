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

#include "nuria/templateloader.hpp"
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
