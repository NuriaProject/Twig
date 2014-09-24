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
#include "nuria/templateengine.hpp"
#include <nuria/debug.hpp>
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
