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

#include "nuria/filetemplateloader.hpp"

#include <QDateTime>
#include <QFile>

namespace Nuria {
class FileTemplateLoaderPrivate {
public:
	
	QVector< QDir > paths;
	QString suffix;
	
};

}

Nuria::FileTemplateLoader::FileTemplateLoader (QObject *parent)
	: TemplateLoader (parent), d_ptr (new FileTemplateLoaderPrivate)
{
	
}

Nuria::FileTemplateLoader::FileTemplateLoader (const QDir &path, QObject *parent)
	: TemplateLoader (parent), d_ptr (new FileTemplateLoaderPrivate)
{
	this->d_ptr->paths.append (path);
}

Nuria::FileTemplateLoader::FileTemplateLoader (const QVector< QDir > &paths, QObject *parent)
        : TemplateLoader (parent), d_ptr (new FileTemplateLoaderPrivate)
{
	this->d_ptr->paths = paths;
}

Nuria::FileTemplateLoader::~FileTemplateLoader () {
	delete this->d_ptr;
}

QVector< QDir > Nuria::FileTemplateLoader::searchPaths () const {
	return this->d_ptr->paths;
}

void Nuria::FileTemplateLoader::addSearchPath (const QDir &path) {
	this->d_ptr->paths.append (path);
}

void Nuria::FileTemplateLoader::setSearchPaths (const QVector< QDir > &paths) const {
	this->d_ptr->paths = paths;
}

QString Nuria::FileTemplateLoader::suffix () const {
	return this->d_ptr->suffix;
}

void Nuria::FileTemplateLoader::setSuffix (const QString &suffix) {
	this->d_ptr->suffix = suffix;
}

QByteArray Nuria::FileTemplateLoader::load (const QString &name) {
	QString path = findTemplatePath (name);
	if (path.isEmpty ()) {
		return QByteArray ();
	}
	
	// 
	QFile file (path);
	if (!file.open (QIODevice::ReadOnly)) {
		return QByteArray ();
	}
	
	// 
	return file.readAll ();
}

bool Nuria::FileTemplateLoader::hasTemplate (const QString &name) {
	return !findTemplatePath (name).isEmpty ();
}

#include <nuria/debug.hpp>
bool Nuria::FileTemplateLoader::hasTemplateChanged (const QString &name, const QDateTime &since) {
	QString path = findTemplatePath (name);
	if (path.isEmpty ()) {
		return true;
	}
	
	// Resources can't have changed.
	if (path.startsWith (QLatin1Char (':'))) {
		return false;
	}
	
	// 
	return (since < QFileInfo (path).lastModified ());
	
}

QString Nuria::FileTemplateLoader::findTemplatePath (const QString &name) const {
	QString fullName = name + this->d_ptr->suffix;
	QString filePath;
	
	for (int i = 0; i < this->d_ptr->paths.length (); i++) {
		if (pathContainsTemplate (this->d_ptr->paths.at (i), fullName, filePath)) {
			return filePath;
		}
		
	}
	
	return QString ();
}

bool Nuria::FileTemplateLoader::pathContainsTemplate (const QDir &path, const QString &name, QString &filePath) const {
	QString relativePath = path.relativeFilePath (name);
	if (relativePath.isEmpty () || relativePath.startsWith (QLatin1String(".."))) {
		return false;
	}
	
#ifdef Q_OS_WIN
	if (relativePath.at (0) != QLatin1Char (':') && relativePath.contains (QLatin1Char(':'))) {
		return false;
	}
#endif
	
	filePath = path.absoluteFilePath (name);
	return QFile::exists (filePath);
}
