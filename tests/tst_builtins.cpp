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

#include "private/templateengine_p.hpp"
#include "private/builtins.hpp"
#include <nuria/debug.hpp>
#include <QtTest/QTest>

using namespace Nuria;

// 
class BuiltinsTest : public QObject {
	Q_OBJECT
private slots:
	
	void initTestCase ();
	
	void test_data ();
	void test ();
	
	void randomWithoutArguments ();
	void randomEmptyString ();
	void randomString ();
	void randomEmptyList ();
	void randomList ();
	void randomEmptyMap ();
	void randomMap ();
	void randomInvalid ();
	
private:
	
	TemplateProgramPrivate *dptr = new TemplateProgramPrivate;
	
};

Q_DECLARE_METATYPE(Nuria::Template::Builtins::Function)

void BuiltinsTest::initTestCase () {
	dptr->locale = QLocale::c ();
	qsrand (1);
}

void BuiltinsTest::test_data () {
	qRegisterMetaType< Template::Builtins::Function > ();
	using namespace Nuria::Template;
	QDateTime date (QDate (2014, 7, 28), QTime (0, 9, 29));
	QDateTime now = QDateTime::currentDateTime ();
	QString dateFormat = "MM/dd/yyyy HH:mm:ss";
	QString dateStr = date.toString (dateFormat);
	QString dateQt = date.toString ();
	
	QVariant intList = QVariant::fromValue (QList< int > ({ 1, 2, 3 }));
	QVariant intMap = QVariant::fromValue (QMap< int, int > ({ { 1, 2 }, { 3, 4 }, { 5, 6 } }));
	
	QVariant stringList = QVariant (QStringList { "a", "b", "c" });
	QVariant anotherStringList = QVariant (QStringList { "d", "e", "f" });
	
	QTest::addColumn< Builtins::Function > ("func");
	QTest::addColumn< QVariant > ("expected");
	QTest::addColumn< QVariantList > ("args");
	
	// 
	QTest::newRow ("Abs positive") << Builtins::Abs << QVariant::fromValue (5.2) << QVariantList { 5.2 };
	QTest::newRow ("Abs negative") << Builtins::Abs << QVariant::fromValue (5.2) << QVariantList { -5.2 };
	
	QTest::newRow ("Batch too few args") << Builtins::Batch << QVariant () << QVariantList { QVariantList { "foo" } };
	QTest::newRow ("Batch less") << Builtins::Batch << QVariant (QVariantList { "foo", "bar", "bar" })
	                             << QVariantList { QVariantList { "foo" }, 3, "bar" };
	QTest::newRow ("Batch enough") << Builtins::Batch << QVariant (QVariantList  { "foo", "bar", "baz" })
	                               << QVariantList { QVariantList { "foo", "bar", "baz" }, 3, "a" };
	
	QTest::newRow ("Capitalize") << Builtins::Capitalize << QVariant ("Foo bar") << QVariantList { "foo bar" };
	
	QTest::newRow ("Cycle negative index") << Builtins::Cycle << QVariant () << QVariantList { intList, -1 };
	QTest::newRow ("Cycle index in list") << Builtins::Cycle << QVariant (2) << QVariantList { intList, 1 };
	QTest::newRow ("Cycle cycled index") << Builtins::Cycle << QVariant (3) << QVariantList { intList, 5 };
	
	QTest::newRow ("Date QDateTime") << Builtins::Date << QVariant (dateStr) << QVariantList { date, dateFormat };
	QTest::newRow ("Date QString") << Builtins::Date << QVariant (dateStr) << QVariantList { dateQt, dateFormat };
	QTest::newRow ("Date no args") << Builtins::Date << QVariant (dptr->locale.toString (now))
	                               << QVariantList { };
	QTest::newRow ("Date one arg") << Builtins::Date << QVariant (dptr->locale.toString (date))
	                               << QVariantList { date };
	
	QTest::newRow ("Default too few args") << Builtins::Default << QVariant () << QVariantList { "a" };
	QTest::newRow ("Default non-empty string") << Builtins::Default << QVariant ("a") << QVariantList { "a", "b" };
	QTest::newRow ("Default empty string") << Builtins::Default << QVariant ("b") << QVariantList { "", "b" };
	QTest::newRow ("Default non-empty list") << Builtins::Default << QVariant (QVariantList { "a" })
	                                         << QVariantList { QVariantList { "a" }, "b" };
	QTest::newRow ("Default empty list") << Builtins::Default << QVariant ("b")
	                                     << QVariantList { QVariantList { }, "b" };
	QTest::newRow ("Default non-empty map") << Builtins::Default << QVariant (QVariantMap { { "a", "A" } })
	                                         << QVariantList { QVariantMap { { "a", "A" } }, "b" };
	QTest::newRow ("Default empty map") << Builtins::Default << QVariant ("b")
	                                     << QVariantList { QVariantMap {  }, "b" };
	QTest::newRow ("Default non-empty sequential") << Builtins::Default << intList << QVariantList { intList, "b" };
	QTest::newRow ("Default empty sequential") << Builtins::Default << QVariant ("b")
	                                     << QVariantList { QVariant::fromValue (QList< int > ()), "b" };
	QTest::newRow ("Default non-empty associative") << Builtins::Default << intMap << QVariantList { intMap, "b" };
	QTest::newRow ("Default empty associative") << Builtins::Default << QVariant ("b")
	                                    << QVariantList { QVariant::fromValue (QMap< int, int > ()), "b" };
	
	QTest::newRow ("Escape default") << Builtins::Escape << QVariant ("&lt;html&gt;")
	                                 << QVariantList { "<html>" };
	QTest::newRow ("Escape html") << Builtins::Escape << QVariant ("&lt;html&gt;")
	                              << QVariantList { "<html>", "html" };
	QTest::newRow ("Escape js") << Builtins::Escape << QVariant ("\\\"\\'\\r\\n")
	                            << QVariantList { "\"'\r\n", "js" };
	QTest::newRow ("Escape css") << Builtins::Escape << QVariant ("\\\"\\'\\r\\n")
	                             << QVariantList { "\"'\r\n", "css" };
	QTest::newRow ("Escape html_attr") << Builtins::Escape << QVariant ("foo&#x20bar")
	                             << QVariantList { "foo bar", "html_attr" };
	QTest::newRow ("Escape invalid") << Builtins::Escape << QVariant (QString ())
	                                 << QVariantList { "<html>", "something" };
	
	QTest::newRow ("First invalid") << Builtins::First << QVariant () << QVariantList { 123 };
	QTest::newRow ("First string") << Builtins::First << QVariant ("a") << QVariantList { "abc" };
	QTest::newRow ("First list") << Builtins::First << QVariant (1) << QVariantList { intList };
	QTest::newRow ("First map") << Builtins::First << QVariant::fromValue (2) << QVariantList { intMap };
	
	// TODO: Write Builtins::Format test when it's implemented
	//QTest::newRow ("Format") << Builtins::Format << QVariant::fromValue () << QVariantList { };
	
	QTest::newRow ("Join no delim") << Builtins::Join << QVariant ("123") << QVariantList { intList };
	QTest::newRow ("Join w/ delim") << Builtins::Join << QVariant ("1,2,3") << QVariantList { intList, "," };
	
	QTest::newRow ("JsonEncode") << Builtins::JsonEncode << QVariant ("[1,2,3]")
	                             << QVariantList { QVariantList { 1, 2, 3 } };
	
	QTest::newRow ("Keys not a map") << Builtins::Keys << QVariant () << QVariantList { intList };
	QTest::newRow ("Keys map") << Builtins::Keys << QVariant (QVariantList { 1, 3, 5 }) << QVariantList { intMap };
	
	QTest::newRow ("Last invalid") << Builtins::Last << QVariant () << QVariantList { 123 };
	QTest::newRow ("Last string") << Builtins::Last << QVariant ("c") << QVariantList { "abc" };
	QTest::newRow ("Last list") << Builtins::Last << QVariant (3) << QVariantList { intList };
	QTest::newRow ("Last map") << Builtins::Last << QVariant::fromValue (6) << QVariantList { intMap };
	
	QTest::newRow ("Length invalid") << Builtins::Length << QVariant (0) << QVariantList { 123 };
	QTest::newRow ("Length string") << Builtins::Length << QVariant (3) << QVariantList { "abc" };
	QTest::newRow ("Length list") << Builtins::Length << QVariant (3) << QVariantList { intList };
	QTest::newRow ("Length map") << Builtins::Length << QVariant (3) << QVariantList { intMap };
	
	QTest::newRow ("Lower invalid") << Builtins::Lower << QVariant ("") << QVariantList { intList };
	QTest::newRow ("Lower string") << Builtins::Lower << QVariant ("abc") << QVariantList { "ABC" };
	
	QTest::newRow ("Nl2Br") << Builtins::Nl2Br << QVariant ("foo<br />bar<br />") << QVariantList { "foo\nbar\n" };
	
	QTest::newRow ("NumberFormat w/o args") << Builtins::NumberFormat << QVariant ("12") << QVariantList { 12.34 };
	QTest::newRow ("NumberFormat zero") << Builtins::NumberFormat << QVariant ("0") << QVariantList { 0 };
	QTest::newRow ("NumberFormat negative") << Builtins::NumberFormat << QVariant ("-12")
	                                        << QVariantList { -12.34 };
	QTest::newRow ("NumberFormat decimal arg") << Builtins::NumberFormat << QVariant ("12.35")
	                                           << QVariantList { 12.345, 2 };
	QTest::newRow ("NumberFormat 2 args") << Builtins::NumberFormat << QVariant ("12|35")
	                                      << QVariantList { 12.345, 2, "|" };
	QTest::newRow ("NumberFormat 3 args") << Builtins::NumberFormat << QVariant ("12-345|68")
	                                      << QVariantList { 12345.678, 2, "|", "-" };
	
	QTest::newRow ("Max") << Builtins::Max << QVariant (3) << QVariantList { 1, 2, 3 };
	QTest::newRow ("Max in list") << Builtins::Max << QVariant (3) << QVariantList { intList };
	QTest::newRow ("Max in map") << Builtins::Max << QVariant (6) << QVariantList { intMap };
	QTest::newRow ("Max in empty list") << Builtins::Max << QVariant () << QVariantList { QVariantList () };
	
	QTest::newRow ("Merge too few args") << Builtins::Merge << QVariant () << QVariantList { intList };
	QTest::newRow ("Merge list") << Builtins::Merge << QVariant (QVariantList { "a", "b", "c", "d", "e", "f" })
	                             << QVariantList { stringList, anotherStringList };
	QTest::newRow ("Merge map") << Builtins::Merge << QVariant (QVariantMap { { "a", "b" }, { "c", "d" } })
	                            << QVariantList {
	                               QVariantMap { { "a", "b" }, { "c", "C" } },
	                               QVariantMap { { "c", "d" } } };
	QTest::newRow ("Merge invalid") << Builtins::Merge << QVariant () << QVariantList { intList, intMap };
	
	QTest::newRow ("Min") << Builtins::Min << QVariant (1) << QVariantList { 1, 2, 3 };
        QTest::newRow ("Min in list") << Builtins::Min << QVariant (1) << QVariantList { intList };
	QTest::newRow ("Min in map") << Builtins::Min << QVariant (2) << QVariantList { intMap };
        QTest::newRow ("Min in empty list") << Builtins::Min << QVariant () << QVariantList { QVariantList () };
	
	QTest::newRow ("Upper invalid") << Builtins::Upper << QVariant ("") << QVariantList { intList };
        QTest::newRow ("Upper string") << Builtins::Upper << QVariant ("ABC") << QVariantList { "abc" };
	
	QTest::newRow ("Range number") << Builtins::Range << QVariant (QVariantList { 1, 2, 3 })
	                               << QVariantList { 1, 3 };
	QTest::newRow ("Range reverse number") << Builtins::Range << QVariant (QVariantList { 3, 2, 1 })
	                                       << QVariantList { 3, 1 };
	QTest::newRow ("Range step") << Builtins::Range << QVariant (QVariantList { 1, 3, 5 })
	                             << QVariantList { 1, 5, 2 };
	QTest::newRow ("Range character") << Builtins::Range << stringList << QVariantList { "a", "c" };
	QTest::newRow ("Range character reverse") << Builtins::Range << QVariant (QVariantList { "c", "b", "a" })
	                                          << QVariantList { "c", "a" };
	QTest::newRow ("Range invalid step") << Builtins::Range << QVariant (QVariantList { })
	                                     << QVariantList { 1, 5, 0 };
	
	// TODO: Write tests for Raw when implemented.
	//QTest::newRow ("Raw") << Builtins::Raw << QVariant::fromValue () << QVariantList { };
	
	QTest::newRow ("Replace") << Builtins::Replace << QVariant ("foo bar baz bar baz")
	                          << QVariantList { "foo A B A B", QVariantMap { { "A", "bar" }, { "B", "baz" } } };
	
	QTest::newRow ("Reverse string") << Builtins::Reverse << QVariant ("cba") << QVariantList { "abc" };
	QTest::newRow ("Reverse list") << Builtins::Reverse << QVariant (QVariantList { 3, 2, 1 })
	                               << QVariantList { intList };
	
	QTest::newRow ("Round default") << Builtins::Round << QVariant (5.3) << QVariantList { 5.257, 1 };
	QTest::newRow ("Round common") << Builtins::Round << QVariant (5.3) << QVariantList { 5.257, 1, "common" };
	QTest::newRow ("Round floor") << Builtins::Round << QVariant (5.2) << QVariantList { 5.257, 1, "floor" };
	QTest::newRow ("Round ceil") << Builtins::Round << QVariant (5.3) << QVariantList { 5.241, 1, "ceil" };
	
	QTest::newRow ("Slice string -1 ?") << Builtins::Slice << QVariant ("d") << QVariantList { "abcd", -1 };
	QTest::newRow ("Slice string 1 ?") << Builtins::Slice << QVariant ("bcd") << QVariantList { "abcd", 1 };
	QTest::newRow ("Slice string -3 -1") << Builtins::Slice << QVariant ("bc") << QVariantList { "abcd", -3, -1 };
	QTest::newRow ("Slice string 3 5") << Builtins::Slice << QVariant ("d") << QVariantList { "abcd", 3, 5 };
	QTest::newRow ("Slice string 1 -1") << Builtins::Slice << QVariant ("bc") << QVariantList { "abcd", 1, -1 };
	QTest::newRow ("Slice list") << Builtins::Slice << QVariant (QVariantList { 2 })
	                             << QVariantList { intList, 1, 1 };
	
	QTest::newRow ("Sort invalid") << Builtins::Sort << QVariant () << QVariantList { intMap };
	QTest::newRow ("Sort list") << Builtins::Sort << QVariant (QVariantList { 1, 2, 3 })
	                            << QVariantList { QVariantList { 3, 2, 1 } };
	
	QTest::newRow ("Split w/ needle") << Builtins::Split << QVariant (QVariantList { "a", "b", "c", "d" })
	                                  << QVariantList { "a,b,c,d", "," };
	QTest::newRow ("Split w/ needle max") << Builtins::Split << QVariant (QVariantList { "a", "b", "c,d" })
	                                  << QVariantList { "a,b,c,d", ",", 3 };
	QTest::newRow ("Split w/o needle") << Builtins::Split << QVariant (QVariantList { "ab", "cd", "e" })
	                                   << QVariantList { "abcde", "", 2 };
	
	QTest::newRow ("StripTags") << Builtins::StripTags << QVariant ("foo bar")
	                            << QVariantList { "<a href=\"#\">foo  bar</a>" };
	
	QTest::newRow ("Title") << Builtins::Title << QVariant ("Foo Bar Baz") << QVariantList { "foo bar baz" };
	
	QTest::newRow ("Trim default") << Builtins::Trim << QVariant ("foo") << QVariantList { "  foo " };
	QTest::newRow ("Trim with mask") << Builtins::Trim << QVariant ("  foo ")
	                                 << QVariantList { ".  foo .;;.", ".;" };
	
	QTest::newRow ("UrlEncode string") << Builtins::UrlEncode << QVariant ("foo%20bar")
	                                   << QVariantList { "foo bar" };
	QTest::newRow ("UrlEncode list") << Builtins::UrlEncode << QVariant ("f%20oo&ba%20r")
	                                 << QVariantList { QVariantList { "f oo", "ba r" } };
	QTest::newRow ("UrlEncode map") << Builtins::UrlEncode << QVariant ("f%20oo=ba%20r&nuria=project")
	                                << QVariantList { QVariantMap { { "f oo", "ba r" }, { "nuria", "project" } } };
	
}

void BuiltinsTest::test () {
	QFETCH(Nuria::Template::Builtins::Function, func);
	QFETCH(QVariant, expected);
	QFETCH(QVariantList, args);
	
	// 
	QVariant result = Template::Builtins::invokeBuiltin (func, args, this->dptr);
	QCOMPARE(result, expected);
}

void BuiltinsTest::randomWithoutArguments () {
	using namespace Nuria::Template;
	QVERIFY(Builtins::invokeBuiltin (Builtins::Random, { }, this->dptr).toInt () != 0);
}

void BuiltinsTest::randomEmptyString () {
	using namespace Nuria::Template;
	QString str;
	QVERIFY(Builtins::invokeBuiltin (Builtins::Random, { str }, this->dptr).toString ().isEmpty ());
	
}

void BuiltinsTest::randomString () {
	using namespace Nuria::Template;
	QString str = "abc";
	QVERIFY(str.contains (Builtins::invokeBuiltin (Builtins::Random, { str }, this->dptr).toString ()));
	
}

void BuiltinsTest::randomEmptyList () {
	using namespace Nuria::Template;
	QVariantList list;
	QVariant variant (list);
	QVERIFY(!Builtins::invokeBuiltin (Builtins::Random, { variant }, this->dptr).isValid ());
	
}

void BuiltinsTest::randomList () {
	using namespace Nuria::Template;
	QVariantList list { 1, 2, 3 };
	QVariant variant (list);
	QVERIFY(list.contains (Builtins::invokeBuiltin (Builtins::Random, { variant }, this->dptr)));
	
}

void BuiltinsTest::randomEmptyMap () {
	using namespace Nuria::Template;
	QVariantMap map;
	QVERIFY(!Builtins::invokeBuiltin (Builtins::Random, { map }, this->dptr).isValid ());
	
}

void BuiltinsTest::randomMap () {
	using namespace Nuria::Template;
	QMap< int, int > map { { 1, 2 }, { 3, 4 }, { 5, 6 } };
	QVariant variant = QVariant::fromValue (map);
	
	QVERIFY(map.values ().contains (Builtins::invokeBuiltin (Builtins::Random, { variant }, this->dptr).toInt ()));
	
}

void BuiltinsTest::randomInvalid () {
	using namespace Nuria::Template;
	QVERIFY(!Builtins::invokeBuiltin (Builtins::Random, { QDate () }, this->dptr).isValid ());
}

QTEST_MAIN(BuiltinsTest)
#include "tst_builtins.moc"
