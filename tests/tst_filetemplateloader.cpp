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

#include "nuria/filetemplateloader.hpp"
#include <nuria/logger.hpp>
#include <QTemporaryFile>
#include <QtTest/QTest>
#include <QThread>

using namespace Nuria;

// 
class FileTemplateLoaderTest : public QObject {
	Q_OBJECT
private slots:
	
	void noSearchPathsFindsNothing ();
	void findTemplateInFirstPath ();
	void findTemplateInSecondPath ();
	void findTemplateWithSuffix ();
	void preventDirectoryTraversal ();
	void verifyTemplateChanged ();
	
};

void FileTemplateLoaderTest::noSearchPathsFindsNothing () {
	FileTemplateLoader loader;
	QVERIFY(!loader.hasTemplate ("foo"));
	QCOMPARE(loader.load ("foo"), QByteArray ());
}

void FileTemplateLoaderTest::findTemplateInFirstPath () {
	FileTemplateLoader loader ({ QDir (":/first"), QDir (":/second") });
	QVERIFY(loader.hasTemplate ("a"));
	QCOMPARE(loader.load ("a"), QByteArray ("A"));
}

void FileTemplateLoaderTest::findTemplateInSecondPath () {
	FileTemplateLoader loader ({ QDir (":/first"), QDir (":/second") });
	QVERIFY(loader.hasTemplate ("b"));
	QCOMPARE(loader.load ("b"), QByteArray ("B"));
}

void FileTemplateLoaderTest::findTemplateWithSuffix () {
	FileTemplateLoader loader ({ QDir (":/first"), QDir (":/second") });
	
	QVERIFY(!loader.hasTemplate ("c"));
	QCOMPARE(loader.load ("c"), QByteArray ());
	
	loader.setSuffix (".twig");
	
	QVERIFY(loader.hasTemplate ("c"));
	QCOMPARE(loader.load ("c"), QByteArray ("C"));
}

void FileTemplateLoaderTest::preventDirectoryTraversal () {
	FileTemplateLoader loader ({ QDir (":/first"), QDir (":/second") });
	QVERIFY(!loader.hasTemplate ("../unreachable"));
	QCOMPARE(loader.load ("../unreachable"), QByteArray ());
}

void FileTemplateLoaderTest::verifyTemplateChanged () {
	FileTemplateLoader loader (QDir::temp ());
	QTemporaryFile tempFile;
	QVERIFY(tempFile.open ());
	
	tempFile.write ("1");
	tempFile.flush ();
	
	// Get relative name of the temp file
	QString fileName = QDir::temp ().relativeFilePath (tempFile.fileName ());
	QVERIFY(!fileName.isEmpty ());
	QVERIFY(loader.hasTemplate (fileName));
	
	// 
	QDateTime now = QDateTime::currentDateTime ();
	QVERIFY(!loader.hasTemplateChanged (fileName, now));
	QThread::sleep (1); // Hack to let the file system catch up.
	tempFile.write ("2");
	tempFile.flush ();
	
	// 
	QDateTime after = QDateTime::currentDateTime ();
	QVERIFY(loader.hasTemplateChanged (fileName, now));
	QVERIFY(!loader.hasTemplateChanged (fileName, after));
	
}

QTEST_MAIN(FileTemplateLoaderTest)
#include "tst_filetemplateloader.moc"
