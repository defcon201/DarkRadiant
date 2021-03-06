#include "RadiantTest.h"

#include "ifilesystem.h"

namespace test
{

using VfsTest = RadiantTest;

TEST_F(VfsTest, FileSystemModule)
{
    // Confirm its module properties
    
    EXPECT_EQ(GlobalFileSystem().getName(), "VirtualFileSystem");
    EXPECT_TRUE(GlobalFileSystem().getDependencies().empty());
}

TEST_F(VfsTest, FilePrerequisites)
{
    // Check presence of some files
    EXPECT_EQ(GlobalFileSystem().getFileCount("nothere"), 0);
    EXPECT_EQ(GlobalFileSystem().getFileCount("materials/example.mtr"), 1);
    EXPECT_EQ(GlobalFileSystem().getFileCount("models/darkmod/test/unit_cube.ase"), 1);
    EXPECT_EQ(GlobalFileSystem().getFileCount("models/darkmod/test/unit_cube_blah.ase"), 0);
    EXPECT_EQ(GlobalFileSystem().getFileCount("models/darkmod/test/unit_cube.lwo"), 1);
}

TEST_F(VfsTest, VisitEntireTree)
{
    // Use a visitor to walk the tree
    std::set<std::string> foundFiles;
    GlobalFileSystem().forEachFile(
        "", "*",
        [&](const vfs::FileInfo& fi) { foundFiles.insert(fi.name); },
        0
    );
    EXPECT_EQ(foundFiles.count("dummy"), 0);
    EXPECT_EQ(foundFiles.count("materials/example.mtr"), 1);
    EXPECT_EQ(foundFiles.count("models/darkmod/test/unit_cube.ase"), 1);
}

TEST_F(VfsTest, VisitMaterialsFolderOnly)
{
    // Visit files only under materials/
    typedef std::map<std::string, vfs::FileInfo> FileInfos;
    FileInfos mtrFiles;
    GlobalFileSystem().forEachFile(
        "materials/", "mtr",
        [&](const vfs::FileInfo& fi) { mtrFiles.insert(std::make_pair(fi.name, fi)); },
        0
    );
    EXPECT_TRUE(!mtrFiles.empty());
}

TEST_F(VfsTest, LeafNamesVsFullPath)
{
    // Visit files only under materials/
    typedef std::map<std::string, vfs::FileInfo> FileInfos;
    FileInfos mtrFiles;
    GlobalFileSystem().forEachFile(
        "materials/", "mtr",
        [&](const vfs::FileInfo& fi) { mtrFiles.insert(std::make_pair(fi.name, fi)); },
        0
    );
    
    // When giving a topdir to visit, the returned file names should be
    // relative to that directory.
    EXPECT_EQ(mtrFiles.count("materials/example.mtr"), 0);
    EXPECT_EQ(mtrFiles.count("example.mtr"), 1);
    EXPECT_EQ(mtrFiles.count("materials/tdm_ai_nobles.mtr"), 0);
    EXPECT_EQ(mtrFiles.count("tdm_ai_nobles.mtr"), 1);

    // But we can reconstruct the original path using the FileInfo::fullPath
    // method.
    EXPECT_EQ(mtrFiles.find("example.mtr")->second.fullPath(), "materials/example.mtr");
    EXPECT_EQ(mtrFiles.find("tdm_ai_nobles.mtr")->second.fullPath(), "materials/tdm_ai_nobles.mtr");
}

TEST_F(VfsTest, forEachFileTrailingSlashInsensitive)
{
    // forEachFile() should work the same regardless of whether we have a
    // trailing slash on the base directory name
    typedef std::map<std::string, vfs::FileInfo> FileInfos;

    FileInfos mtrFiles;
    GlobalFileSystem().forEachFile(
        "materials/", "mtr",
        [&](const vfs::FileInfo& fi) { mtrFiles.insert(std::make_pair(fi.name, fi)); },
        0
    );

    FileInfos withoutSlash;
    GlobalFileSystem().forEachFile(
        "materials", "mtr",
        [&](const vfs::FileInfo& fi) { withoutSlash.insert(std::make_pair(fi.name, fi)); },
        0
    );

    EXPECT_EQ(withoutSlash.size(), mtrFiles.size());
    EXPECT_TRUE(std::equal(withoutSlash.begin(), withoutSlash.end(), mtrFiles.begin()));
}

TEST_F(VfsTest, assetsLstFileHandling)
{
    // Visit models dir and store visibility information
    std::map<std::string, vfs::Visibility> fileVis;
    GlobalFileSystem().forEachFile(
        "models/", "*", [&](const vfs::FileInfo& fi) { fileVis[fi.name] = fi.visibility; },
        0
    );

    EXPECT_TRUE(!fileVis.empty());

    EXPECT_EQ(fileVis.count("darkmod/test/unit_cube.ase"), 1);
    EXPECT_EQ(fileVis["darkmod/test/unit_cube.ase"], vfs::Visibility::HIDDEN);
    EXPECT_EQ(fileVis["darkmod/test/unit_cube.lwo"], vfs::Visibility::NORMAL);

    // The assets.lst should be converted into visibility information, but NOT
    // returned as an actual file to the calling code.
    EXPECT_EQ(fileVis.count("assets.lst"), 0);
}

}
