#pragma once

#include "../../../includes.h"

#include "../../../mosst/string.h"


namespace MOSS { namespace Disk {
	class Partition;
namespace FileSystem {


class FileSystemBase;

class ObjectBase {
	public:
		FileSystemBase*const filesystem;

		MOSST::String const path;

		enum Type {
			TYPE_FILE,
			TYPE_DIRECTORY
		} const type;

	protected:
		inline ObjectBase(FileSystemBase* filesystem, char const* path, Type type) : filesystem(filesystem), path(path), type(type) {}
	public:
		inline virtual ~ObjectBase(void) {}
};
class ObjectFile final : public ObjectBase {
	public:
		inline ObjectFile(FileSystemBase* filesystem, char const* path) : ObjectBase(filesystem,path,TYPE_FILE) {}
		inline virtual ~ObjectFile(void) {}

		MOSST::Vector<uint8_t>* get_new_data(void) const {
			assert_term(false,"Not implemented!");
			return nullptr;
		}
		void save_data(MOSST::Vector<uint8_t> const& /*data*/) {
			assert_term(false,"Not implemented!");
		}
};
class ObjectDirectory final : public ObjectBase {
	public:
		MOSST::Vector<MOSST::String> paths_children;

	public:
		inline ObjectDirectory(FileSystemBase* filesystem, char const* path) : ObjectBase(filesystem,path,TYPE_DIRECTORY) {}
		inline virtual ~ObjectDirectory(void) {}

		ObjectBase* get_new_child(MOSST::String const& path_child) const;
};

class FileSystemBase {
	public:
		Partition*const partition;

		ObjectDirectory* root;

	protected:
		inline explicit FileSystemBase(Partition* partition) : partition(partition) {
			root = new ObjectDirectory(this,"/");
		}
	public:
		inline virtual ~FileSystemBase(void) {
			delete root;
		}

		//TODO:
		//virtual File* read_new(char const* path) = 0;
		//virtual void write(const File* file) = 0;

		virtual ObjectBase* get_new_child(ObjectDirectory const* directory, MOSST::String const& path_child) const = 0;

	private:
		void _helper_print(ObjectDirectory* directory, int depth) const;
	public:
		inline void print(void) const { _helper_print(root,0); }
};


}}}
