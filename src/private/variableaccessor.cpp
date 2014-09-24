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

#include "variableaccessor.hpp"

#include <QAssociativeIterable>
#include <QSequentialIterable>
#include <nuria/metaobject.hpp>
#include <QMetaProperty>

static Nuria::MetaObject *metaObjectForType (const QByteArray &typeName) {
	Nuria::MetaObject *meta = Nuria::MetaObject::byName (typeName);
	if (!meta && typeName.endsWith ('*')) {
		return metaObjectForType (typeName.left (typeName.length () - 1));
	}
	
	return meta;
}

bool Nuria::Template::VariableAcessor::walkChain (QVariant &cur, const QVariantList &chain, int index) {
	
	if (index >= chain.length ()) {
		return true;
	}
	
	// 
	int type = cur.userType ();
	
	// List access
	if (type == QMetaType::QVariantList) {
		return walkList (cur, chain, index);
	}
	
	if (cur.canConvert< QVariantList > ()) {
		return walkListType (cur, chain, index);
	}
	
	// Map access
	if (type == QMetaType::QVariantMap) {
		return walkMap (cur, chain, index);
	}
	
	if (cur.canConvert< QVariantMap > ()) {
		return walkMapType (cur, chain, index);
	}
	
	if (type == QMetaType::QVariantMap) {
		return walkMap (cur, chain, index);
	}
	
	// MetaObject type
	MetaObject *meta = metaObjectForType (QMetaType::typeName (type));
	if (meta) {
		return walkMetaObject (meta, cur, chain, index);
	}
	
	// QObject
	if (QMetaType::typeFlags (type) & QMetaType::PointerToQObject) {
		return walkQObject (cur, chain, index);
	}
	
	// Unknown
	return false;
}

bool Nuria::Template::VariableAcessor::walkList (QVariant &cur, const QVariantList &chain, int index) {
	const QVariant &name = chain.at (index);
	QVariantList list = cur.toList ();
	bool ok = false;
	
	int at = name.toInt (&ok);
	if (!ok || at < 0 || at >= list.length ()) {
		// TODO: Output error
		return false;
	}
	
	// 
	cur = list.at (at);
	return walkChain (cur, chain, index + 1);
	
}

bool Nuria::Template::VariableAcessor::walkMap (QVariant &cur, const QVariantList &chain, int index) {
	const QVariant &name = chain.at (index);
	QVariantMap map = cur.toMap ();
	
	QString key = name.toString ();
	if (key.isEmpty () || !map.contains (key)) {
		// TODO: Output error
		return false;
	}
	
	// 
	cur = map.value (key);
	return walkChain (cur, chain, index + 1);
	
}

bool Nuria::Template::VariableAcessor::walkListType (QVariant &cur, const QVariantList &chain, int index) {
	QSequentialIterable iter = cur.value< QSequentialIterable > ();
	const QVariant &name = chain.at (index);
	bool ok = false;
	
	// Get index
	int at = name.toInt (&ok);
	if (!ok || at < 0 || at >= iter.size ()) {
		// TODO: Output error
		return false;
	}
	
	// 
	cur = *(iter.begin () + at);
	return walkChain (cur, chain, index + 1);
}

bool Nuria::Template::VariableAcessor::walkMapType (QVariant &cur, const QVariantList &chain, int index) {
	QAssociativeIterable iter = cur.value< QAssociativeIterable > ();
	const QVariant &name = chain.at (index);
	
	// Get key
	cur = iter.value (name);
	if (!cur.isValid ()) {
		// TODO: Output error
		return false;
	}
	
	// 
	return walkChain (cur, chain, index + 1);
	
}

bool Nuria::Template::VariableAcessor::walkMetaObject (MetaObject *meta, QVariant &cur,
                                                       const QVariantList &chain, int index) {
	QByteArray name = chain.at (index).toString ().toLatin1 ();
	void *object = const_cast< void * > (cur.constData ());
	if (name.isEmpty () || !object) {
		return false;
	}
	
	// 
	
	// See if it's a field
	MetaField field = meta->fieldByName (name);
	if (field.isValid ()) {
		cur = field.read (object);
		return walkChain (cur, chain, index + 1);
	}
	
	// Function?
	int begin = meta->methodLowerBound (name);
//	int end = meta->methodUpperBound (name);
	
	if (begin >= 0) {
		// TODO: Warn if there are multiple functions with this name
		cur = QVariant::fromValue (meta->method (begin).callback (object));
		return walkChain (cur, chain, index + 1);
	}
	
	// Not found.
	return false;
}

bool Nuria::Template::VariableAcessor::walkQObject (QVariant &cur, const QVariantList &chain, int index) {
	QByteArray name = chain.at (index).toString ().toLatin1 ();
	QObject *object = cur.value< QObject * > ();
	if (name.isEmpty () || !object) {
		return false;
	}
	
	// Only properties are supported right now.
	// TODO: Should QObjects support functions?
	const QMetaObject *meta = object->metaObject ();
	int idx = meta->indexOfProperty (name.constData ());
	
	if (idx < 0) {
		// TODO: Issue error
		return false;
	}
	
	// 
	cur = meta->property (idx).read (object);
	return walkChain (cur, chain, index + 1);
}
