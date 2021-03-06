#pragma once

#include <map>
#include <string>
#include <set>
#include "TreeModel.h"
#include <wx/dataview.h>

namespace wxutil
{

/**
 * Utility class to construct a TreeModel from a series of string paths
 * in the form "models/first/second/object.lwo" or similar. The class accepts
 * the Tree Store and then string paths, one by one, adding each one to the
 * tree in the appropriate place.
 *
 * Since the VFSTreePopulator has no knowledge of the column data to be inserted
 * into the tree, it does not set any of the values but merely calls
 * TreeStore::AddItem() to get an iterator pointing to the newly-added
 * row. In order to insert the data, the calling code must then call the
 * visitor function which will provide the visitor with two objects - the
 * created iterator and the full input string which the visitor can then
 * use to populate the data appropriately.
 *
 * When the VFSTreePopulator is destroyed it will free any temporary structures
 * used during tree creation, such as the hashtable of iterator objects
 * used to locate parent nodes. The TreeModel will not be destroyed since it
 * is owned by the calling code.
 */
class VFSTreePopulator
{
	// The tree store to populate
    TreeModel::Ptr _store;

	// Toplevel node to add children under
	wxDataViewItem _topLevel;

	// Maps of names to corresponding treemodel iterators, for both intermediate
	// paths and explicitly presented paths
	typedef std::map<std::string, wxDataViewItem> NamedIterMap;
	NamedIterMap _iters;

	// Set of paths that are passed in through addPath(), to distinguish them
	// from intermediate constructed paths
	std::set<std::string> _explicitPaths;

public:
	/**
	 * Construct a VFSTreePopulator which will populate the given tree store.
	 *
	 * @param store
	 * Tree store to populate.
	 *
	 * @param toplevel
	 * TreeModel::iterator pointing to the toplevel node, under which all paths should
	 * be added. Default is empty to indicate that paths should be added under
	 * the tree root.
	 */
	VFSTreePopulator(const TreeModel::Ptr& store, const wxDataViewItem& toplevel = wxDataViewItem());

	/** Destroy the VFSTreePopulator and all temporary data.
	 */
	virtual ~VFSTreePopulator();

	/** Add a single VFS string to the tree, which will be split automatically
	 * and inserted at the correct place in the tree.
	 */
	void addPath(const std::string& path);

    // Column population function. The inserted Row will be passed as argument.
    typedef std::function<void(TreeModel::Row& row, 
                               const std::string& leafName, 
                               bool isFolder)> ColumnPopulationCallback;

    /**
     * Like addPath() above this method also takes a function object as 
     * argument to allow for immediate population of the TreeStore's column values. 
     * As soon as the item has been created in the correct spot of the tree 
     * the callback will be invoked passing the TreeModel::Row as argument.
     *
     * Client code needs to decide between this or the regular addPath() method
     * and cannot mix between these two, as this variant here does not keep track
     * of the "path-is-explicit" property and therefore doesn't support the
     * forEachNode() method being called with a Visitor.
     *
     * Note: the callback needs to invoke row.SendItemAdded() when done.
     */
    void addPath(const std::string& path, const ColumnPopulationCallback& func);

	/** Visitor interface.
	 */
	struct Visitor
	{
		virtual ~Visitor() {}

		/**
		 * Visit callback function, called for each node in the tree.
		 *
		 * @param store
		 * The tree store to insert into.
		 *
		 * @param row
		 * The row to be processed.
		 *
		 * @param path
		 * Full VFS path of the current node, as presented to addPath().
		 *
		 * @param isExplicit
		 * true if the path was explicitly inserted via addPath(), false if the
		 * path was created as an intermediate parent of another explicit path.
		 */
		virtual void visit(TreeModel& store,
						   wxutil::TreeModel::Row& row,
						   const std::string& path,
						   bool isExplicit) = 0;
	};

	/** 
     * Visit each node in the constructed tree, passing the wxDataViewItem and
	 * the VFS string to the visitor object so that data can be inserted.
     * Note that this method is not meant to be used if the addPath() overload
     * taking a ColumnPopulationCallback has been used earlier on.
	 */
	void forEachNode(Visitor& visitor);

private:
    // Main recursive add function. Accepts a VFS path and adds the new node,
    // calling itself if necessary to add all of the parent directories, then
    // returns a wxDataViewItem pointing to the new node.
    const wxDataViewItem& addRecursive(const std::string& path, 
                                       const ColumnPopulationCallback& func,
                                       int recursionLevel = 0);
};

} // namespace
