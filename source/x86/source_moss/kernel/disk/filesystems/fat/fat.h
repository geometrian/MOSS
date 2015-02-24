#pragma once

#include "../../../../includes.h"

#include "../filesystem.h"


namespace MOSS { namespace Disk { namespace FileSystem {


class FileSystemFAT final : public FileSystemBase {
	private:

	public:
		explicit FileSystemFAT(Partition* partition) : FileSystemBase(partition) {
		}
		virtual ~FileSystemFAT(void) {
		}
};


}}}