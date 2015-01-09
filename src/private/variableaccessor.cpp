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
