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
#include <QtTest/QtTest>

using namespace Nuria;

// 
class TemplateEngineCachingTest : public QObject {
	Q_OBJECT
private slots:
	
	void onTemplateChangedSignal ();
	void onTemplateChangedSignalInDependencies ();
	void onAllTemplatesChangedSignal ();
	void loaderHasTemplateChangedCheck ();
	
};

void TemplateEngineCachingTest::onTemplateChangedSignal () {
	TemplateEngine engine;
	
	engine.render ("a");
	engine.render ("b");
	QVERIFY(engine.isTemplateInCache ("a"));
	QVERIFY(engine.isTemplateInCache ("b"));
	
	emit engine.loader ()->templateChanged ("a");
	
	QVERIFY(!engine.isTemplateInCache ("a"));
	QVERIFY(engine.isTemplateInCache ("b"));
}

void TemplateEngineCachingTest::onTemplateChangedSignalInDependencies () {
	TemplateEngine engine;
	MemoryTemplateLoader *loader = new MemoryTemplateLoader;
	engine.setLoader (loader);
	
	loader->addTemplate ("a", "a{% include 'b' %}");
	loader->addTemplate ("b", "b");
	
	engine.render ("a");
	
	emit engine.loader ()->templateChanged ("b");
	QVERIFY(!engine.isTemplateInCache ("a"));
	
}

void TemplateEngineCachingTest::onAllTemplatesChangedSignal () {
	TemplateEngine engine;
	
	engine.render ("a");
	engine.render ("b");
	QVERIFY(engine.isTemplateInCache ("a"));
	QVERIFY(engine.isTemplateInCache ("b"));
	
	emit engine.loader ()->allTemplatesChanged ();
	
	QVERIFY(!engine.isTemplateInCache ("a"));
	QVERIFY(!engine.isTemplateInCache ("b"));
	
}

class NonCachingLoader : public TemplateLoader {
public:
	
	bool loadCalled = false;
	
	QByteArray load (const QString &name) override {
		loadCalled = true;
		return name.toUtf8 ();
	}
	
	bool hasTemplateChanged (const QString &, const QDateTime &) override {
		return true;
	}
	
};

void TemplateEngineCachingTest::loaderHasTemplateChangedCheck () {
	TemplateEngine engine;
	NonCachingLoader *loader = new NonCachingLoader;
	engine.setLoader (loader);
	
	engine.render ("a");
	loader->loadCalled = false;
	
	engine.render ("a");
	QVERIFY(loader->loadCalled);
	
}

QTEST_MAIN(TemplateEngineCachingTest)
#include "tst_templateengine_caching.moc"
