#include "filesystem.h"

#include "../../kernel.h"


namespace MOSS { namespace Disk { namespace FileSystem {


ObjectBase* ObjectDirectory::get_new_child(MOSST::String const& path_child) const {
	return filesystem->get_new_child(this,path_child);
}


void FileSystemBase::_helper_print(ObjectDirectory* directory, int depth) const {
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


}}}
