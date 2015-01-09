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

#ifndef NURIA_FILETEMPLATELOADER_HPP
#define NURIA_FILETEMPLATELOADER_HPP

#include "templateloader.hpp"
#include "twig_global.hpp"
#include <QVector>
#include <QDir>

namespace Nuria {

class FileTemplateLoaderPrivate;

/**
 * \brief A template loader which loads templates from files.
 * 
 * This template loader tries to load files from one or more paths in the
 * file-system. This includes paths in Qt resources.
 * 
 * Search paths are tried from the one added first to the one added last.
 * You can also define a common file suffix, which is appended to every
 * template name internally. Use this if all your templates have a common
 * suffix, like ".twig", and don't want to write it in the Twig code.
 * 
 * Directory traversal attacks, meaning traversing outside of the given
 * search paths, are not possible.
 * 
 * \note If template names are case-sensitive or not depends on the platform.
 */
class NURIA_TWIG_EXPORT FileTemplateLoader : public TemplateLoader {
	Q_OBJECT
public:
	
	/** Default constructor. */
	explicit FileTemplateLoader (QObject *parent = 0);
	
	/** Constructor which uses \a path as search path. */
	explicit FileTemplateLoader (const QDir &path, QObject *parent = 0);
	
	/** Contstructor which uses \a paths as search paths. */
	explicit FileTemplateLoader (const QVector< QDir > &paths, QObject *parent = 0);
	
	/** Destructor. */
	~FileTemplateLoader () override;
	
	/** Returns the current search paths. */
	QVector< QDir > searchPaths () const;
	
	/** Adds \a path to the list of search paths. */
	void addSearchPath (const QDir &path);
	
	/** Replaces the current search path list with \a paths. */
	void setSearchPaths (const QVector< QDir > &paths) const;
	
	/** Returns the file suffix. */
	QString suffix () const;
	
	/**
	 * Sets the file suffix, which is appended internally to every template
	 * name.
	 */
	void setSuffix (const QString &suffix);
	
	// 
	QByteArray load (const QString &name) override;
	bool hasTemplate (const QString &name) override;
	bool hasTemplateChanged (const QString &name, const QDateTime &since) override;
	
private:
	QString findTemplatePath (const QString &name) const;
	bool pathContainsTemplate (const QDir &path, const QString &name, QString &filePath) const;
	
	FileTemplateLoaderPrivate *d_ptr;
	
};

}

#endif // NURIA_FILETEMPLATELOADER_HPP
