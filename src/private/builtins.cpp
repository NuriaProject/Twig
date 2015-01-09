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

#include "builtins.hpp"

#include <QAssociativeIterable>
#include "templateengine_p.hpp"
#include <QSequentialIterable>
#include <QRegularExpression>
#include <QJsonDocument>
#include "astnodes.hpp"
#include <QDateTime>
#include <climits>
#include <cmath>
#include <QUrl>

#include <nuria/callback.hpp>

Nuria::Template::Builtins::Function Nuria::Template::Builtins::nameLookup (const QString &name) {
	static const QMap< QString, Function > g_builtins = {
	        { QStringLiteral("abs"), Abs },
		{ QStringLiteral("batch"), Batch },
		{ QStringLiteral("capitalize"), Capitalize },
	        { QStringLiteral("cycle"), Cycle },
		{ QStringLiteral("date"), Date },
//		{ QStringLiteral("date_modify"), DateModify },
		{ QStringLiteral("default"), Default },
	        { QStringLiteral("dump"), Dump },
		{ QStringLiteral("e"), Escape },
		{ QStringLiteral("escape"), Escape },
		{ QStringLiteral("first"), First },
		{ QStringLiteral("format"), Format },
		{ QStringLiteral("join"), Join },
		{ QStringLiteral("json_encode"), JsonEncode },
		{ QStringLiteral("keys"), Keys },
		{ QStringLiteral("last"), Last },
		{ QStringLiteral("length"), Length },
		{ QStringLiteral("lower"), Lower },
	        { QStringLiteral("max"), Max },
		{ QStringLiteral("merge"), Merge },
	        { QStringLiteral("min"), Min },
		{ QStringLiteral("nl2br"), Nl2Br },
		{ QStringLiteral("number_format"), NumberFormat },
	        { QStringLiteral("random"), Random },
	        { QStringLiteral("range"), Range },
		{ QStringLiteral("raw"), Raw },
		{ QStringLiteral("replace"), Replace },
		{ QStringLiteral("reverse"), Reverse },
		{ QStringLiteral("round"), Round },
		{ QStringLiteral("slice"), Slice },
		{ QStringLiteral("sort"), Sort },
		{ QStringLiteral("split"), Split },
		{ QStringLiteral("striptags"), StripTags },
		{ QStringLiteral("title"), Title },
		{ QStringLiteral("trim"), Trim },
		{ QStringLiteral("upper"), Upper },
		{ QStringLiteral("url_encode"), UrlEncode },
	        { QStringLiteral("block"), Block },
	        { QStringLiteral("parent"), Parent }
	};
	
	return g_builtins.value (name, Unknown);
}

bool Nuria::Template::Builtins::isBuiltinConstant (Function func) {
	switch (func) {
	case Block:
	case Date: // Locale
	case Dump:
	case NumberFormat: // Locale
	case Parent:
	case Random:
		return false;
		
		// All others are constant.
	default: return true;
	}
	
}

QVariant Nuria::Template::Builtins::invokeBuiltin (Function func, const QVariantList &args,
                                                   TemplateProgramPrivate *dptr) {
	if (args.isEmpty () && func != Date && func != Dump && func != Random) {
		return QVariant ();
	}
	
	// 
	switch (func) {
	case Unknown:
	case Parent: // Handled by MethodCallValueNode::compile()
	default:
		return QVariant ();
	
	case Abs: return fabs (args.first ().toDouble ());
	case Batch: return filterBatch (args);
	case Capitalize: return filterCapitalize (args);
	case Cycle: return filterCycle (args);
	case Date: return filterDate (dptr, args);
//	case DateModify: return filterDateModify (args);
	case Default: return filterDefault (args);
	case Dump: return functionDump (dptr, args);
	case Escape: return filterEscape (dptr, args);
	case First: return filterFirst (args);
	case Format: return filterFormat (args);
	case Join: return filterJoin (args);
	case JsonEncode: return filterJsonEncode (args);
	case Keys: return filterKeys (args);
	case Last: return filterLast (args);
	case Length: return filterLength (args);
	case Lower: return filterLower (args);
	case Nl2Br: return filterNl2Br (args);
	case NumberFormat: return filterNumberFormat (dptr, args);
	case Max: return functionMax (args);
	case Merge: return filterMerge (args);
	case Min: return functionMin (args);
	case Upper: return filterUpper (args);
	case Random: return functionRandom (args);
	case Range: return functionRange (args);
	case Raw: return filterRaw (args);
	case Replace: return filterReplace (args);
	case Reverse: return filterReverse (args);
	case Round: return filterRound (args);
	case Slice: return filterSlice (args);
	case Sort: return filterSort (args);
	case Split: return filterSplit (args);
	case StripTags: return filterStripTags (args);
	case Title: return filterTitle (args);
	case Trim: return filterTrim (args);
	case UrlEncode: return filterUrlEncode (args);
	case Block: return functionBlock (dptr, args);
	}
	
}

QString Nuria::Template::Builtins::escape (QString data, EscapeMode mode) {
	static const QByteArray included = QByteArrayLiteral("-._~");
	
	switch (mode) {
	case EscapeMode::Verbatim: return QString ();
	case EscapeMode::Html: return data.toHtmlEscaped ();
	case EscapeMode::JavaScript:
	case EscapeMode::Css:
		data.replace (QLatin1Char ('"'), QLatin1String ("\\\""));
		data.replace (QLatin1Char ('\''), QLatin1String ("\\'"));
		data.replace (QLatin1Char ('\r'), QLatin1String ("\\r"));
		data.replace (QLatin1Char ('\n'), QLatin1String ("\\n"));
		data.replace (QLatin1Char ('\t'), QLatin1String ("\\t"));
		return data;
	case EscapeMode::Url:
		return QString::fromLatin1 (QUrl::toPercentEncoding (data));
	case EscapeMode::HtmlAttr:
		return QString::fromLatin1 (data.toUtf8 ().toPercentEncoding (QByteArray (), included)
		                            .replace ('%', "&#x"));
	}
	
	// 
	return QString ();
	
}

QVariant Nuria::Template::Builtins::filterBatch (const QVariantList &args) {
	if (args.length () < 3) return QVariant ();
	QVariantList list = args.at (0).toList ();
	int count = args.at (1).toInt ();
	const QVariant &item = args.at (2);
	
	// Is the list already long enough?
	if (list.length () >= count) {
		return list;
	}
	
	// Fill the list with additional elements
	QVariantList copy (list);
	copy.reserve (count);
	
	int toAdd = count - list.length ();
	for (int i = 0; i < toAdd; i++) {
		copy.append (item);
	}
	
	return copy;
}

QVariant Nuria::Template::Builtins::filterCapitalize (const QVariantList &args) {
	QString string = args.first ().toString ();
	string[0] = string[0].toUpper ();
	return string;
}

QVariant Nuria::Template::Builtins::filterCycle (const QVariantList &args) {	
	if (args.length () < 2) {
		return QVariant ();
	}
	
	// 
	const QVariant &list = args.at (0);
	int index = args.at (1).toInt ();
	
	QSequentialIterable iter = list.value< QSequentialIterable > ();
	if (iter.size () < 1 || index < 0) {
		return QVariant ();
	}
	
	return iter.at (index % iter.size ());
	
}

QVariant Nuria::Template::Builtins::filterDate (TemplateProgramPrivate *dptr, const QVariantList &args) {
	QDateTime dateTime;
	
	// Get datetime
	if (args.length () > 0) {
		if (args.first ().userType () == QMetaType::QString) {
			// TODO: Implement PHPs strtotime() for relative time calculations
			dateTime = QDateTime::fromString (args.first ().toString ());
		} else {
			dateTime = args.first ().toDateTime ();
		}
		
	} else {
		dateTime = QDateTime::currentDateTime ();
	}
	
	// Use custom format?
	if (args.length () > 1) {
		QString format = args.at (1).toString ();
		return dateTime.toString (format);
	}
	
	// Use locale format
	return dptr->locale.toString (dateTime);
}

QVariant Nuria::Template::Builtins::filterDefault (const QVariantList &args) {
	if (args.length () < 2) return QVariant ();
	
	const QVariant &value = args.first ();
	if (!value.isValid ()) return args.at (1);
	
	// Empty check
	int type = value.userType ();
	if (type == QMetaType::QString) {
		if (!value.toString ().isEmpty ()) {
			return value;
		}
		
	} else if (value.canConvert< QVariantList > ()) {
		QSequentialIterable it = value.value< QSequentialIterable > ();
		if (it.size () > 0) {
			return value;
		}
		
	} else if (value.canConvert< QVariantMap > ()) {
		QAssociativeIterable it = value.value< QAssociativeIterable > ();
		if (it.size () > 0) {
			return value;
		}
		
	}
	
	// Value is empty.
	return args.at (1);
}

static void dumpVariable (const QVariant &variable, QString &into) {
	QDebug printer (&into);
	printer << variable;
}

static void dumpEnvironment (Nuria::TemplateProgramPrivate *dptr, QString &into) {
	QVariantMap map;
	for (int i = 0; i < dptr->values.length (); i++) {
		map.insert (dptr->variables.at (i), dptr->values.at (i));
	}
	
	// 
	dumpVariable (map, into);
}

QVariant Nuria::Template::Builtins::functionDump (TemplateProgramPrivate *dptr, const QVariantList &args) {
	QString result;
	
	if (args.isEmpty ()) {
		dumpEnvironment (dptr, result);
	} else {
		for (int i = 0; i < args.length (); i++) {
			dumpVariable (args.at (i), result);
			result.append (", ");
		}
		
		result.chop (3); // Trailing " , "
	}
	
	// 
	return result;
}

Nuria::EscapeMode Nuria::Template::Builtins::parseEscapeMode (const QString &name) {
	using namespace Nuria;
	
	if (name.isEmpty () || name == QLatin1String ("html")) {
		return EscapeMode::Html;
	}
	
	if (name == QLatin1String ("js")) {
		return EscapeMode::JavaScript;
	}
	
	if (name == QLatin1String ("css")) {
		return EscapeMode::Css;
	}
	
	
	if (name == QLatin1String ("url")) {
		return EscapeMode::Url;
	}
	
	
	if (name == QLatin1String ("html_attr")) {
		return EscapeMode::HtmlAttr;
	}
	
	return EscapeMode::Verbatim;
}

QVariant Nuria::Template::Builtins::filterEscape (TemplateProgramPrivate *dptr, const QVariantList &args) {
	QString modeName;
	
	if (args.length () > 1) {
		modeName = args.at (1).toString ();
	}
	
	// Parse escape mode
	EscapeMode mode = parseEscapeMode (modeName);
	if (mode == EscapeMode::Verbatim) {
		// TODO: Location information for the error.
		dptr->error = TemplateError (TemplateError::Renderer, TemplateError::InvalidEscapeMode,
		                             QStringLiteral("Invalid escape mode '%1'").arg (modeName));
		return QString ();
	}
	
	// Don't double-escape if we're in a autoescape block
	if (mode == dptr->escapeMode) {
		return args.first ();
	}
	
	// 
	QString data = args.first ().toString ();
	return escape (data, mode);
	
}

static QVariant firstOrLast (const QVariant &input, bool first) {
	int type = input.userType ();
	
	// String
	if (type == QMetaType::QString) {
		QString str = input.toString ();
		return first ? str.left (1) : str.right (1);
	}
	
	// List
	if (input.canConvert< QVariantList > ()) {
		QSequentialIterable it = input.value< QSequentialIterable > ();
		if (it.size () == 0) return QVariant ();
		return first ? it.at (0) : it.at (it.size () - 1);
	}
	
	// Maps
	if (input.canConvert< QVariantMap > ()) {
		QAssociativeIterable it = input.value< QAssociativeIterable > ();
		if (it.size () == 0) return QVariant ();
		return *(first ? it.begin () : it.end () - 1);
	}
	
	// Unknown
	return QVariant ();
	
}

QVariant Nuria::Template::Builtins::filterFirst (const QVariantList &args) {
	return firstOrLast (args.first (), true);
}

QVariant Nuria::Template::Builtins::filterFormat (const QVariantList &args) {
	Q_UNUSED(args);
	return QVariant ();
}

QVariant Nuria::Template::Builtins::filterJoin (const QVariantList &args) {
	QSequentialIterable it = args.first ().value< QSequentialIterable > ();
	QString delim;
	
	if (args.length () > 1) {
		delim = args.at (1).toString ();
	}
	
	// Join
	QString result;
	int length = it.size ();
	for (int i = 0; i < length; i++) {
		result.append (it.at (i).toString ());
		if (i + 1 < length) result.append (delim);
	}
	
	// Done.
	return result;
}

QVariant Nuria::Template::Builtins::filterJsonEncode (const QVariantList &args) {
	QByteArray data = QJsonDocument::fromVariant (args.first ()).toJson (QJsonDocument::Compact);
	return QString::fromUtf8 (data);
}

static QVariantList mapKeys (const QVariant &map) {
	QAssociativeIterable iter = map.value< QAssociativeIterable > ();
	QVariantList keys;
	keys.reserve (iter.size ());
	
	// Iterate over map
	auto it = iter.begin ();
	auto end = iter.end ();
	for (; it != end; ++it) {
		keys.append (it.key ());
	}
	
	return keys;
}

QVariant Nuria::Template::Builtins::filterKeys (const QVariantList &args) {
	const QVariant &map = args.first ();
	if (!map.canConvert< QVariantMap > ()) {
		return QVariant ();
	}
	
	return mapKeys (map);
}

QVariant Nuria::Template::Builtins::filterLast (const QVariantList &args) {
	return firstOrLast (args.first (), false);
}

QVariant Nuria::Template::Builtins::filterLength (const QVariantList &args) {
	const QVariant &input = args.first ();
	int type = input.userType ();
	
	// String
	if (type == QMetaType::QString) {
		return input.toString ().length ();
	}
	
	// List
	if (input.canConvert< QVariantList > ()) {
		QSequentialIterable it = input.value< QSequentialIterable > ();
		return it.size ();
	}
	
	// Maps
	if (input.canConvert< QVariantMap > ()) {
		QAssociativeIterable it = input.value< QAssociativeIterable > ();
		return it.size ();
	}
	
	return 0;
}

QVariant Nuria::Template::Builtins::filterLower (const QVariantList &args) {
	return args.first ().toString ().toLower ();
}

QVariant Nuria::Template::Builtins::filterNl2Br (const QVariantList &args) {
	QString string = args.first ().toString ();
	string.replace (QLatin1Char ('\n'), QLatin1String ("<br />"));
	return string;
}

QVariant Nuria::Template::Builtins::filterNumberFormat (TemplateProgramPrivate *dptr, const QVariantList &args) {
	double num = args.first ().toDouble ();
	
	// Count digits before comma
	int digits = 0;
	int number = num;
	do {
		number /= 10;
		digits++;
	} while (number != 0);
	
	// 
	int decimals = 0;
	if (args.length () > 1) decimals = args.at (1).toInt ();
	QString numStr = dptr->locale.toString (num, 'g', decimals + digits);
	
	// TODO: Fails if the third argument is used and the second argument is equal to the group separator.
	if (args.length () > 2) {
		numStr.replace (dptr->locale.decimalPoint (), args.at (2).toString ());
	}
	
	// 
	if (args.length () > 3) {
		numStr.replace (dptr->locale.groupSeparator (), args.at (3).toString ());
	}
	
	// 
	return numStr;
}

static QVariantList getListFromArguments (const QVariantList &args) {
	if (args.first ().canConvert< QVariantList > ()) {
		return args.first ().value< QVariantList > ();
	} else if (args.first ().canConvert< QVariantMap > ()) {
		return args.first ().value< QVariantMap > ().values ();
	}
	
	return args;
	
}

static QVariant minMaxOfList (const QVariantList &list, bool min) {
	QVariant cur = list.first ();
	if (min) {
		for (int i = 1; i < list.length (); i++) {
			if (list.at (i) < cur) cur = list.at (i);
		}
	} else {
		for (int i = 1; i < list.length (); i++) {
			if (list.at (i) > cur) cur = list.at (i);
		}
	}
	
	// 
	return cur;
}

QVariant Nuria::Template::Builtins::functionMax (const QVariantList &args) {
	QVariantList list = getListFromArguments (args);
	if (list.isEmpty ()) {
		return QVariant ();
	}
	
	// 
	return minMaxOfList (list, false);
}

static QVariantList mergeLists (const QVariant &left, const QVariant &right) {
	QVariantList list = left.toList ();
	QSequentialIterable iter = right.value< QSequentialIterable > ();
	
	list.reserve (list.size () + iter.size ());
	for (int i = 0; i < iter.size (); i++) {
		list.append (iter.at (i));
	}
	
	return list;
}

static QVariantMap mergeMaps (const QVariant &left, const QVariant &right) {
	QVariantMap map = left.toMap ();
	QAssociativeIterable iter = right.value< QAssociativeIterable > ();
	
	auto it = iter.begin ();
	auto end = iter.end ();
	for (; it != end; ++it) {
		map.insert (it.key ().toString (), it.value ());
	}
	
	return map;
}

QVariant Nuria::Template::Builtins::filterMerge (const QVariantList &args) {
	if (args.length () < 2) return QVariant ();
	
	// Maps
	if (args.at (0).canConvert< QVariantMap > () &&
	    args.at (1).canConvert< QVariantMap > ()) {
		return mergeMaps (args.at (0), args.at (1));
	}
	
	// Lists
	if (args.at (0).canConvert< QVariantList > () &&
	    args.at (1).canConvert< QVariantList > ()) {
		return mergeLists (args.at (0), args.at (1));
	}
	
	// Fail
	return QVariant ();
}

QVariant Nuria::Template::Builtins::functionMin (const QVariantList &args) {
	QVariantList list = getListFromArguments (args);
	if (list.isEmpty ()) {
		return QVariant ();
	}
	
	// 
	return minMaxOfList (list, true);
}

QVariant Nuria::Template::Builtins::filterUpper (const QVariantList &args) {
	return args.first ().toString ().toUpper ();
}

QVariant Nuria::Template::Builtins::functionRandom (const QVariantList &args) {
	if (args.isEmpty ()) return qrand ();
	
	// Integer as argument
	if (args.first ().userType () == QMetaType::Int ||
	    args.first ().userType () == QMetaType::Double) {
		int barrier = args.first ().toInt ();
		return (barrier != 0) ? qrand () % barrier : 0;
	}
	
	// String as argument
	if (args.first ().userType () == QMetaType::QString) {
		QString str = args.first ().toString ();
		return (str.length () > 0) ? QString (str.at (qrand () % str.length ())) : QString ();
	}
	
	// List as argument
	if (args.first ().canConvert< QVariantList > ()) {
		QSequentialIterable iter = args.first ().value< QSequentialIterable > ();
		return (iter.size () > 0) ? iter.at (qrand () % iter.size ()) : QVariant ();
	}
	
	// Map as argument
	if (args.first ().canConvert< QVariantMap > ()) {
		QAssociativeIterable iter = args.first ().value< QAssociativeIterable > ();
		return (iter.size () > 0) ? (iter.begin () + (qrand () % iter.size ())).value () : QVariant ();
	}
	
	// Unknown
	return QVariant ();
}

static void createNumberRange (QVariantList &list, double start, double max, double step) {
	if (start > max) {
		step = std::min (step, -step);
		for (double cur = start; cur >= max; cur += step)
			list.append (cur);
	} else {
		step = std::max (step, -step);
		for (double cur = start; cur <= max; cur += step)
			list.append (cur);
	}
	
}

static bool normalizeChar (char &character) {
	if (isdigit (character)) character -= '0';
	else if (isalpha (character)) character -= ((character >= 'a') ? 'a' - 10 : 'A' - 36);
	else return false;
	return true;
}

static void createCharRange (QVariantList &list, char start, char max, int step) {
	const static char lookup[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (!normalizeChar (start) || !normalizeChar (max) ||
	    start > sizeof(lookup) || max > sizeof(lookup)) {
		return;
	}
	
	// 
	if (start > max) {
		step = std::min (step, -step);
		for (char cur = start; cur >= max; cur += step)
			list.append (QString (QLatin1Char (lookup[cur])));
	} else {
		step = std::max (step, -step);
		for (char cur = start; cur <= max; cur += step)
			list.append (QString (QLatin1Char (lookup[cur])));
	}
	
}

QVariant Nuria::Template::Builtins::functionRange (const QVariantList &args) {
	if (args.length () < 2) return QVariant ();
	
	// Read arguments
	const QVariant &from = args.at (0);
	const QVariant &max = args.at (1);
	QVariant step = 1;
	
	if (args.length () > 2) step = args.at (2);
	
	// Check range type.
	QVariantList result;
	if (from.userType () == QMetaType::QString) {
		// Character range
		QString fromStr = from.toString ();
		QString maxStr = max.toString ();
		int stepInt = step.toInt ();
		if (!fromStr.isEmpty () && !maxStr.isEmpty () && stepInt != 0) {
			createCharRange (result, fromStr.at (0).toLatin1 (),
			                 maxStr.at (0).toLatin1 (), stepInt);
		}
		
	} else {
		// Number range
		double fromNum = from.toDouble ();
		double maxNum = max.toDouble ();
		double stepNum = step.toDouble ();
		if (stepNum != 0.0) {
			createNumberRange (result, fromNum, maxNum, stepNum);
		}
		
	}
	
	// Done.
	return result;
}

QVariant Nuria::Template::Builtins::filterRaw (const QVariantList &args) {
	return QVariant ();
}

QVariant Nuria::Template::Builtins::filterReplace (const QVariantList &args) {
	if (args.length () < 2) return QVariant ();
	
	QString string = args.first ().toString ();
	QVariantMap map = args.at (1).toMap ();
	
	auto it = map.constBegin ();
	auto end = map.constEnd ();
	for (; it != end; ++it) {
		string.replace (it.key (), it.value ().toString ());
	}
	
	return string;
}

QVariant Nuria::Template::Builtins::filterReverse (const QVariantList &args) {
	
	// String
	if (args.first ().userType () == QMetaType::QString) {
		QString string = args.first ().toString ();
		std::reverse (string.begin (), string.end ());
		return string;
	}
	
	// List
	if (args.first ().canConvert< QVariantList > ()) {
		QSequentialIterable iter = args.first ().value< QSequentialIterable > ();
		QVariantList result;
		result.reserve (iter.size ());
		
		for (int i = iter.size () - 1; i >= 0; i--) {
			result.append (iter.at (i));
		}
		
		return result;
	}
	
	// Unsupported
	return QVariant ();
	
}

QVariant Nuria::Template::Builtins::filterRound (const QVariantList &args) {
	double number = args.first ().toDouble ();
	int precision = 0;
	QString mode;
	
	if (args.length () > 1) precision = args.at (1).toInt ();
	if (args.length () > 2) mode = args.at (2).toString ();
	
	// 
#ifdef __GLIBC__
	int div = pow10 (precision);
#else
	int div = powf (10.0, precision);
#endif
	if (mode.isEmpty () || mode == QLatin1String ("common")) {
		return round (number * div) / div;
	} else if (mode == QLatin1String ("ceil")) {
		return ceil (number * div) / div;
	} else if (mode == QLatin1String ("floor")) {
		return floor (number * div) / div;
	}
	
	// 
	return QVariant ();
	
}

static void calculateStartLength (int &start, int &length, int size) {
	if (start < 0) {
		start = size + start;
	}
	
	if (length < 0) {
		length = size + length - start;
	}
	
}

QVariant Nuria::Template::Builtins::filterSlice (const QVariantList &args) {
	if (args.length () < 2) return QVariant ();
	const QVariant &data = args.first ();
	int start = args.at (1).toInt ();
	int length = INT_MAX;
	
	if (args.length () > 2) length = args.at (2).toInt ();
	
	// String
	if (data.userType () == QMetaType::QString) {
		QString string = data.toString ();
		calculateStartLength (start, length, string.length ());
		return string.mid (start, length);
	}
	
	// List
	if (data.canConvert< QVariantList > ()) {
		QSequentialIterable iter = data.value< QSequentialIterable > ();
		calculateStartLength (start, length, iter.size ());
		
		QVariantList result;
		result.reserve (length);
		
		int end = std::min (start + length, iter.size ());
		for (int i = start; i < end; i++) {
			result.append (iter.at (i));
		}
		
		return result;
	}
	
	// Unsupported
	return QVariant ();
	
}

QVariant Nuria::Template::Builtins::filterSort (const QVariantList &args) {
	if (!args.first ().canConvert< QVariantList > ()) {
		return QVariant ();
	}
	
	// 
	QVariantList list = args.first ().toList ();
	std::sort (list.begin (), list.end ());
	return list;
}

QVariant Nuria::Template::Builtins::filterSplit (const QVariantList &args) {
	if (args.first ().userType () != QMetaType::QString || args.length () < 2) {
		return QVariant ();
	}
	
	// 
	int maxLength = INT_MAX;
	if (args.length () > 2) maxLength = args.at (2).toInt ();
	
	// 
	QString string = args.at (0).toString ();
	QString delim = args.at (1).toString ();
	QStringList list;
	
	if (delim.isEmpty () && maxLength < INT_MAX) {
		// Split after each 'maxLength' characters.
		int chars = std::max (1, maxLength);
		int sections = string.size () / chars;
		
		if (string.size () % chars) {
			sections++;
		}
		
		// 
		list.reserve (sections);
		for (int i = 0; i < sections; i++) {
			list.append (string.mid (i * chars, chars));
		}
		
	} else {
		// Split by needle
		list = string.split (delim);
		
	}
	
	// Maximum length
	if (!delim.isEmpty () && list.length () > maxLength) {
		QString last = QStringList (list.mid (maxLength - 1)).join (delim);
		list.erase (list.begin () + maxLength, list.end ());
		list.last () = last;
	}
	
	// 
	QVariantList result;
	result.reserve (list.length ());
	for (int i = 0; i < list.length (); i++) {
		result.append (list.at (i));
	}
	
	return result;
}

QVariant Nuria::Template::Builtins::filterStripTags (const QVariantList &args) {
	static QRegularExpression rx (QStringLiteral("<[^>]*>"));
	QString string = args.first ().toString ();
	
	string.replace (rx, QString ());
	return string.simplified ();
}

QVariant Nuria::Template::Builtins::filterTitle (const QVariantList &args) {
	
	// Make sure we have a string
	QString string = args.first ().toString ();
	if (string.isEmpty ()) {
		return string;
	}
	
	// 
	int pos = -1;
	int length = string.length ();
	do {
		pos++;
		if (pos < length) {
			string[pos] = string[pos].toTitleCase ();
		}
		
	} while (pos < length && (pos = string.indexOf (QLatin1Char (' '), pos)) != -1);
	
	// 
	return string;
}

QVariant Nuria::Template::Builtins::filterTrim (const QVariantList &args) {
	QString string = args.first ().toString ();
	
	if (args.length () > 1) {
		QString mask = args.at (1).toString ();
		
		int begin = 0;
		int end = string.length () - 1;
		
		for (; mask.contains (string.at (begin)) && begin < string.length (); begin++);
		for (; mask.contains (string.at (end)) && end >= 0; end--);
		
		if (begin > end) {
			return QString ();
		}
		
		return string.mid (begin, end - begin + 1);
		
	}
	
	// Default trim
	return string.trimmed ();
}

static QString stringToUrlEncoded (const QString &data) {
	return QString::fromLatin1 (QUrl::toPercentEncoding (data));
}

static QVariant listToUrlEncoded (const QVariant &list) {
	QSequentialIterable iter = list.value< QSequentialIterable > ();
	auto it = iter.begin ();
	auto end = iter.end ();
	
	QString result;
	for (; it != end; ++it) {
		result.append (stringToUrlEncoded ((*it).toString ()));
		result.append (QLatin1Char ('&'));
	}
	
	result.chop (1);
	return result;
}

static QVariant mapToUrlEncoded (const QVariant &list) {
	QAssociativeIterable iter = list.value< QAssociativeIterable > ();
	auto it = iter.begin ();
	auto end = iter.end ();
	
	QString result;
	for (; it != end; ++it) {
		result.append (stringToUrlEncoded (it.key ().toString ()));
		result.append (QLatin1Char ('='));
		result.append (stringToUrlEncoded (it.value ().toString ()));
		result.append (QLatin1Char ('&'));
	}
	
	result.chop (1);
	return result;
}

QVariant Nuria::Template::Builtins::filterUrlEncode (const QVariantList &args) {
	if (args.first ().canConvert< QVariantList > ()) {
		return listToUrlEncoded (args.first ());
	} else if (args.first ().canConvert< QVariantMap > ()) {
		return mapToUrlEncoded (args.first ());
	}
	
	return stringToUrlEncoded (args.first ().toString ());
}

QVariant Nuria::Template::Builtins::functionBlock (TemplateProgramPrivate *dptr, const QVariantList &args) {
	BlockNode *block = dptr->root->blocks.value (args.first ().toString ().toUtf8 ());
	
	if (!block) {
		return QString ();
	}
	
	return block->render (dptr);
}
