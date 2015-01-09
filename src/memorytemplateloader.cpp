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

namespace Nuria {
class MemoryTemplateLoaderPrivate {
public:
	
	MemoryTemplateLoader::Map map;
	
};
}

Nuria::MemoryTemplateLoader::MemoryTemplateLoader (QObject *parent)
	: TemplateLoader (parent), d_ptr (new MemoryTemplateLoaderPrivate)
{
	
}

Nuria::MemoryTemplateLoader::MemoryTemplateLoader (Nuria::MemoryTemplateLoader::Map map, QObject *parent)
	: TemplateLoader (parent), d_ptr (new MemoryTemplateLoaderPrivate)
{
	
	this->d_ptr->map = map;
	
}

Nuria::MemoryTemplateLoader::~MemoryTemplateLoader () {
	delete this->d_ptr;
}

Nuria::MemoryTemplateLoader::Map Nuria::MemoryTemplateLoader::map () const {
	return this->d_ptr->map;
}

void Nuria::MemoryTemplateLoader::setMap (const Map &map) {
	this->d_ptr->map = map;
	emit allTemplatesChanged ();
}

void Nuria::MemoryTemplateLoader::addTemplate (const QString &name, const QByteArray &data) {
	bool alreadyKnown = this->d_ptr->map.contains (name);
	this->d_ptr->map.insert (name, data);
	
	if (alreadyKnown) {
		emit templateChanged (name);
	}
	
}

void Nuria::MemoryTemplateLoader::removeTemplate (const QString &name) {
	this->d_ptr->map.remove (name);
	emit templateChanged (name);
}

QByteArray Nuria::MemoryTemplateLoader::load (const QString &name) {
	return this->d_ptr->map.value (name);
}

bool Nuria::MemoryTemplateLoader::hasTemplate (const QString &name) {
	return this->d_ptr->map.contains (name);
}
