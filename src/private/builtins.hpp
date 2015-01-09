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

#ifndef NURIA_TEMPLATE_BUILTINS_HPP
#define NURIA_TEMPLATE_BUILTINS_HPP

#include "../nuria/templateerror.hpp"
#include "templateengine_p.hpp"
#include <QVariant>
#include <QObject>

namespace Nuria {

class TemplateProgramPrivate;

namespace Template {

class Node;

/**
 * \internal
 * \brief Internal class providing implementations for built-in Twig functions.
 */
class Builtins {
public:
	
	/**
	 * Built-in functions.
	 * See: http://twig.sensiolabs.org/doc/filters/index.html
	 */
	enum Function {
		Unknown = 0,
		Abs,
		Batch,
		Capitalize,
		Cycle,
		Date,
//		DateModify,
		Default,
		Dump,
		Escape,
		First,
		Format,
		Join,
		JsonEncode,
		Keys,
		Last,
		Length,
		Lower,
		Nl2Br,
		NumberFormat,
		Max,
		Merge,
		Min,
		Upper,
		Random,
		Range,
		Raw,
		Replace,
		Reverse,
		Round,
		Slice,
		Sort,
		Split,
		StripTags,
		Title,
		Trim,
		UrlEncode,
		Block,
		Parent
	};
	
	// 
	static Function nameLookup (const QString &name);
	static bool isBuiltinConstant (Function func);
	static QVariant invokeBuiltin (Function func, const QVariantList &args,
	                               TemplateProgramPrivate *dptr);
	
	// 
	static QString escape (QString data, EscapeMode mode);
	static EscapeMode parseEscapeMode (const QString &name);
	
	// 
	static QVariant filterBatch (const QVariantList &args);
	static QVariant filterCapitalize (const QVariantList &args);
	static QVariant filterCycle (const QVariantList &args);
	static QVariant filterAbs (const QVariantList &args);
	static QVariant functionBlock (TemplateProgramPrivate *dptr, const QVariantList &args);
	static QVariant filterDate (TemplateProgramPrivate *dptr, const QVariantList &args);
//	static QVariant filterDateModify (const QVariantList &args);
	static QVariant filterDefault (const QVariantList &args);
	static QVariant functionDump (TemplateProgramPrivate *dptr, const QVariantList &args);
	static QVariant filterEscape (TemplateProgramPrivate *dptr, const QVariantList &args);
	static QVariant filterFirst (const QVariantList &args);
	static QVariant filterFormat (const QVariantList &args);
	static QVariant filterJoin (const QVariantList &args);
	static QVariant filterJsonEncode (const QVariantList &args);
	static QVariant filterKeys (const QVariantList &args);
	static QVariant filterLast (const QVariantList &args);
	static QVariant filterLength (const QVariantList &args);
	static QVariant filterLower (const QVariantList &args);
	static QVariant filterNl2Br (const QVariantList &args);
	static QVariant filterNumberFormat (TemplateProgramPrivate *dptr, const QVariantList &args);
	static QVariant functionMax (const QVariantList &args);
	static QVariant filterMerge (const QVariantList &args);
	static QVariant functionMin (const QVariantList &args);
	static QVariant filterUpper (const QVariantList &args);
	static QVariant functionRandom (const QVariantList &args);
	static QVariant functionRange (const QVariantList &args);
	static QVariant filterRaw (const QVariantList &args);
	static QVariant filterReplace (const QVariantList &args);
	static QVariant filterReverse (const QVariantList &args);
	static QVariant filterRound (const QVariantList &args);
	static QVariant filterSlice (const QVariantList &args);
	static QVariant filterSort (const QVariantList &args);
	static QVariant filterSplit (const QVariantList &args);
	static QVariant filterStripTags (const QVariantList &args);
	static QVariant filterTitle (const QVariantList &args);
	static QVariant filterTrim (const QVariantList &args);
	static QVariant filterUrlEncode (const QVariantList &args);
	
};

}
}

#endif // NURIA_TEMPLATE_BUILTINS_HPP
