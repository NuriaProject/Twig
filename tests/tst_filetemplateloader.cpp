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

#include "nuria/filetemplateloader.hpp"
#include <nuria/debug.hpp>
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
