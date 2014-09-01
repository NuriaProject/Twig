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
