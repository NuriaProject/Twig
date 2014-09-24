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
