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

#ifndef NURIA_TEMPLATENGINE_PRIVATE_HPP
#define NURIA_TEMPLATENGINE_PRIVATE_HPP

#include "../nuria/templateerror.hpp"
#include <nuria/callback.hpp>
#include "astnodes.hpp"
#include <QSharedData>
#include <QDateTime>
#include <QVariant>
#include <QVector>
#include <QCache>
#include <memory>

namespace Nuria {
namespace Template {
class SharedNode;
class BlockNode;
class Tokenizer;
class Compiler;
class Parser;
class Node;
}

class TemplateProgramPrivate;
class TemplateProgram;
class TemplateEngine;
class TemplateLoader;

struct Function {
	
	Function (Callback cb = Callback (), bool constant = false)
	        : callback (cb), isConstant (constant) { }
	
	Callback callback;
	bool isConstant = false;
};

typedef QMap< QString, Function > FunctionMap;

class TemplateEnginePrivate {
public:
	
	TemplateEngine *q_ptr = nullptr;
	
	TemplateError lastError;
	Template::Parser *parser;
	Template::Tokenizer *tokenizer;
	Template::Compiler *renderer;
	TemplateLoader *loader;
	QVariantMap values;
	FunctionMap functions;
	QLocale locale;
	
	// For tracking of variable changes between cached programs and the
	// engine.
	int versionId = 0;
	
	QCache< QString, TemplateProgram > cache;
	
};

// Escape modes for expansion rendering
// See: http://twig.sensiolabs.org/doc/filters/escape.html
enum class EscapeMode {
	Verbatim = 0, // No escaping is done
	Html, // HTML code
	JavaScript, // JS string
	Css, // CSS data
	Url, // URI or URL parameters
	HtmlAttr // HTML attributes
};

// Records for variable accesses.
struct VariableUsage {
	
	// Default arguments to make QVector happy
	VariableUsage (Template::Location loc = Template::Location (),
	               bool writing = false, bool constant = false)
	        : location (loc), isConstant (constant), isWriting (writing)
	{}
	
	Template::Location location;
	bool isConstant = false;
	bool isWriting = false;
};

// Trimming options for whitespace around a node
namespace Trim {
enum {
	None = 0,
	Left = 1,
	Right = 2,
	InnerLeft = 4,
	InnerRight = 8
};
}

typedef QVector< VariableUsage > VariableUsageList;
struct CompileInformation {
	
	
	int conditionBranchDepth = 0;
	Template::BlockNode *currentParentBlock = nullptr;
	
	QMap< Template::Node *, int > trim;
	
};

class TemplateProgramPrivate : public QSharedData {
public:
	
	QExplicitlySharedDataPointer< Template::SharedNode > root;
	mutable TemplateError error;
	QDateTime compiledAt;
	QLocale locale;
	
	// List of needed templates
	QStringList dependencies;
	
	// Currently active escape mode. Needed at runtime for escape().
	EscapeMode escapeMode = EscapeMode::Verbatim;
	bool spaceless = false;
	
	QStringList variables;
	QVector< QVariant > values;
	FunctionMap functions;
	int versionId = -1;
	
	// Variable usage book-keeping
	QVector< VariableUsageList > usages;
	
	// Only used during compilation
	CompileInformation *info = nullptr;
	
	// 
	int addOrGetVariablePosition (const QString &name) {
		int idx = variables.indexOf (name);
		if (idx < 0) {
			idx = variables.length ();
			variables.append (name);
			values.append (QVariant ());
			this->usages.append (VariableUsageList ());
		}
		
		return idx;
	}
	
	// 
	void addUsageRecord (int variableId, Template::Location location,
	                     bool writeAccess = false, bool isConstant = false) {
		this->usages[variableId].append (VariableUsage (location, writeAccess, isConstant));
	}
	
	void prependWriteUsageRecord (int variableId, Template::Location location) {
		this->usages[variableId].prepend (VariableUsage (location, true, false));
	}
	
	bool isFirstUsageRecordWriting (int variableId) const {
		const VariableUsageList &usages = this->usages.at (variableId);
		return (!usages.isEmpty () && usages.first ().isWriting);
	}
	
	// Transfers trimming information from 'prev' -> 'now'. Returns 'now'.
	Template::Node *transferTrim (Template::Node *prev, Template::Node *now) {
		if (prev != now && prev && now ) {
			int mode = info->trim.take (prev);
			
			if (mode) {
				info->trim.insert (now, mode);
			}
			
		}
		
		return now;
	}
	
};

}

#endif // NURIA_TEMPLATENGINE_PRIVATE_HPP
