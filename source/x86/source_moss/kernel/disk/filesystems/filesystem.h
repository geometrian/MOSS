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

		MOSST::String const name;

		enum Type {
			TYPE_FILE,
			TYPE_DIRECTORY
		} const type;

	protected:
		inline ObjectBase(FileSystemBase* filesystem, char const* name, Type type) : filesystem(filesystem), name(name), type(type) {}
	public:
		inline virtual ~ObjectBase(void) {}
};
class ObjectFileBase : public ObjectBase {
	protected:
		inline ObjectFileBase(FileSystemBase* filesystem, char const* name) : ObjectBase(filesystem,name,TYPE_FILE) {}
	public:
		inline virtual ~ObjectFileBase(void) {}

		#if 0
		MOSST::Vector<uint8_t>* get_new_data(void) const {
			assert_term(false,"Not implemented!");
			return nullptr;
		}
		void save_data(MOSST::Vector<uint8_t> const& /*data*/) {
			assert_term(false,"Not implemented!");
		}
		#endif
};
class ObjectDirectoryBase : public ObjectBase {
	public:
		bool is_loaded;
		MOSST::Vector<ObjectBase*> children;

	protected:
		inline ObjectDirectoryBase(FileSystemBase* filesystem, char const* name) : ObjectBase(filesystem,name,TYPE_DIRECTORY), is_loaded(false) {}
	public:
		virtual ~ObjectDirectoryBase(void);

		virtual void load_entries(void) = 0;

		virtual ObjectBase* get_new_child(MOSST::String const& child_name) const = 0;
};

class FileSystemBase {
	public:
		Partition*const partition;

		ObjectDirectoryBase* root;

	protected:
		//Note: derived class must allocate root directory!
		inline explicit FileSystemBase(Partition* partition) : partition(partition) {}
	public:
		inline virtual ~FileSystemBase(void) {
			delete root;
		}

		//TODO:
		//virtual File* read_new(char const* path) = 0;
		//virtual void write(const File* file) = 0;

	/*private:
		void _helper_print(ObjectDirectoryBase* directory, int depth) const;
	public:
		inline void print(void) const { _helper_print(root,0); }*/
};


}}}
