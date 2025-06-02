#include "filesystem.hpp"

#include "../../kernel.hpp"


namespace MOSS { namespace Disk { namespace FileSystem {


ObjectDirectoryBase::~ObjectDirectoryBase(void) {
	for (int i=0;i<children.size;++i) {
		delete children[i];
	}
}


/*void FileSystemBase::_helper_print(ObjectDirectoryBase* directory, int depth) const {
	for (int i=0;i<directory->children.size;++i) {
		ObjectBase* child = directory->get_new_child(directory->paths_children[i]);

		for (int i=0;i<depth;++i) kernel->write("  ");
		kernel->write("\"%s\"\n",child->name.c_str());

		switch (child->type) {
			case ObjectBase::Type::TYPE_FILE:
				break;
			case ObjectBase::Type::TYPE_DIRECTORY:
				_helper_print(static_cast<ObjectDirectoryBase*>(child),depth+1);
				break;
		}

		delete child;
	}
}*/


}}}
