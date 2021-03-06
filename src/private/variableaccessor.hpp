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

#ifndef NURIA_TEMPLATE_VARIABLEACCESSOR_HPP
#define NURIA_TEMPLATE_VARIABLEACCESSOR_HPP

#include <QVariant>

namespace Nuria {

class MetaObject;

namespace Template {

class Node;

/**
 * \internal
 * \brief Internal class for accessing variables.
 */
class VariableAcessor {
public:
	
	static bool walkChain (QVariant &cur, const QVariantList &chain, int index);
	static bool walkList (QVariant &cur, const QVariantList &chain, int index);
	static bool walkMap (QVariant &cur, const QVariantList &chain, int index);
	static bool walkListType (QVariant &cur, const QVariantList &chain, int index);
	static bool walkMapType (QVariant &cur, const QVariantList &chain, int index);
	static bool walkMetaObject (MetaObject *meta, QVariant &cur, const QVariantList &chain, int index);
	static bool walkQObject (QVariant &cur, const QVariantList &chain, int index);
	
private:
	
	// No way to construct this class.
	VariableAcessor () = delete;
	
};

}
}

#endif // NURIA_TEMPLATE_VARIABLEACCESSOR_HPP
