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
#include "nuria/templateengine.hpp"
#include <nuria/logger.hpp>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QtTest/QTest>
#include <QDir>

#define TESTS_PATH_PREFIX ":/test-cases"

using namespace Nuria;

struct TestCase {
	QString name;
	QVariantMap variables;
	QVariant templateData;
	QString output;
	QString errorComponent;
	QString errorCode;
	bool skip = false;
};

Q_DECLARE_METATYPE(TestCase)

// 
class TemplateEngineTest : public QObject {
	Q_OBJECT
private slots:
	
	void run_data ();
	void run ();
	
private:
	QString testNameFromPath (const QString &path);
	const char *getTagFromPath (const QString &path);
	QVariantMap loadTestCaseFile (const QString &path);
	TestCase parseTestCase (const QString &path);
	
	QVector< QByteArray > m_tags;
	
};

static QStringList listOfTests () {
	QStringList list;
	
	QDir testDir (TESTS_PATH_PREFIX);
	QStringList files = testDir.entryList ({ "*.json" });
	
	for (const QString &cur : files) {
		list.append (testDir.absoluteFilePath (cur));
	}
	
	return list;
}

QString TemplateEngineTest::testNameFromPath (const QString &path) {
	return path.section (QLatin1Char ('/'), -1).section (QLatin1Char ('.'), 0, 0);
}

const char *TemplateEngineTest::getTagFromPath (const QString &path) {
	this->m_tags.append (testNameFromPath (path).toLatin1 ());
	return this->m_tags.last ().constData ();
}

QVariantMap TemplateEngineTest::loadTestCaseFile (const QString &path) {
	QFile file (path);
	if (!file.open (QIODevice::ReadOnly)) {
		qCritical() << "Failed to open test-case" << path;
		std::terminate ();
	}
	
	// 
	QJsonParseError error;
	QByteArray jsonData = file.readAll ();
	QJsonDocument doc = QJsonDocument::fromJson (jsonData, &error);
	
	if (error.error != QJsonParseError::NoError) {
		qCritical() << "JSON error in test-case" << path << ":" << error.errorString ()
		            << "near position" << error.offset << jsonData.mid (error.offset - 3, 7);
		std::terminate ();
	}
	
	// 
	return doc.toVariant ().toMap ();
}

TestCase TemplateEngineTest::parseTestCase (const QString &path) {
	QVariantMap data = loadTestCaseFile (path);
	TestCase testCase;
	
	testCase.name = testNameFromPath (path);
	
	testCase.variables = data.value (QStringLiteral("variables")).toMap ();
	testCase.templateData = data.value (QStringLiteral("template"));
	testCase.output = data.value (QStringLiteral("output")).toString ();
	testCase.skip = data.value (QStringLiteral("skip")).toBool ();
	
	QString error = data.value (QStringLiteral("error")).toString ();
	testCase.errorComponent = error.section (QLatin1Char (':'), 0, 0);
	testCase.errorCode = error.section (QLatin1Char (':'), 1, 1);
	
	return testCase;
}

void TemplateEngineTest::run_data () {
	qRegisterMetaType< TestCase > ();
	QTest::addColumn< TestCase > ("testCase");
	
	for (const QString &testName : listOfTests ()) { 
		TestCase test = parseTestCase (testName);
		const char *tag = getTagFromPath (testName);
		QTest::newRow (tag) << test;
	}
	
}

void TemplateEngineTest::run () {
	QFETCH(TestCase, testCase);
	
	qDebug() << "Running" << qPrintable(testCase.name);
	
	// Skipped?
	if (testCase.skip) {
		QSKIP("Skipping as marked in test-case");
	}
	
	// 
	MemoryTemplateLoader *loader = new MemoryTemplateLoader;
	TemplateEngine engine;
	
	// Add template(s)
	if (testCase.templateData.userType () == QMetaType::QVariantMap) {
		QVariantMap map = testCase.templateData.toMap ();
		
		auto it = map.constBegin ();
		auto end = map.constEnd ();
		for (; it != end; ++it) {
			loader->addTemplate (it.key (), it.value ().toString ().toUtf8 ());
		}
		
	} else {
		loader->addTemplate ("main", testCase.templateData.toString ().toUtf8 ());
	}
	
	// Apply environment
	engine.setValues (testCase.variables);
	
	// Run
	engine.setLoader (loader);
	QString result = engine.render ("main");
	
	// Verify
	if (testCase.errorCode.isEmpty ()) {
		// Happy path
		if (result != testCase.output || engine.lastError ().hasFailed ()) {
			qWarning() << "Result  :" << result;
			qWarning() << "Expected:" << testCase.output;
			qWarning() << "Error   :" << engine.lastError ();
			QFAIL("Result did not match expected output.");
		}
	} else {
		// Failure path
		TemplateError error = engine.lastError ();
		if (error.componentName () != testCase.errorComponent ||
		    error.errorName () != testCase.errorCode) {
			qWarning() << "Result:" << result;
			qWarning() << "Expected error:" << testCase.errorComponent << "->" << testCase.errorCode;
			qWarning() << "But got error :" << error;
			QFAIL("Expected error did not occur.");
		}
		
	}
	
}

QTEST_MAIN(TemplateEngineTest)
#include "tst_templateengine.moc"
