#pragma once

#include "../../../includes.h"

#include "../../../mosst/string.h"


namespace MOSS { namespace Disk {
	class Partition;
namespace FileSystem {


class ObjectBase {
	public:
		MOSST::String const path;

		enum Type {
			TYPE_FILE,
			TYPE_DIRECTORY
		} const type;

	protected:
		inline ObjectBase(char const* path, Type type) : path(path), type(type) {}
	public:
		inline virtual ~ObjectBase(void) {}
};
class ObjectFile final : public ObjectBase {
	public:
		inline explicit ObjectFile(char const* path) : ObjectBase(path,TYPE_FILE) {}
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
		inline explicit ObjectDirectory(char const* path) : ObjectBase(path,TYPE_DIRECTORY) {}
		inline virtual ~ObjectDirectory(void) {}

		ObjectBase* get_new_child(MOSST::String const& /*path_child*/) const {
			assert_term(false,"Not implemented!");
			return nullptr;
		}
};

class FileSystemBase {
	public:
		Partition*const partition;

		ObjectDirectory* root;

	protected:
		inline explicit FileSystemBase(Partition* partition) : partition(partition) {
			root = new ObjectDirectory("/");
		}
	public:
		inline virtual ~FileSystemBase(void) {
			delete root;
		}

		//TODO:
		//virtual File* read_new(char const* path) = 0;
		//virtual void write(const File* file) = 0;

	private:
		void _helper_print(ObjectDirectory* directory, int depth) const {
			for (int i=0;i<directory->paths_children.size;++i) {
				ObjectBase* child = directory->get_new_child(directory->paths_children[i]);

				for (int i=0;i<depth;++i) kernel->write("  ");
				kernel->write("\"%s\"\n",child->path.c_str());

				switch (child->type) {
					case ObjectBase::Type::TYPE_FILE:
						break;
					case ObjectBase::Type::TYPE_DIRECTORY:
						_helper_print(static_cast<ObjectDirectory*>(child),depth+1);
						break;
				}

				delete child;
			}
		}
	public:
		inline void print(void) const { _helper_print(root,0); }
};


}}}
