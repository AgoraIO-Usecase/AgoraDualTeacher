//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//

#include <stddef.h>

#include "utils/files/file_path.h"

namespace agora {
namespace utils {

#if defined(FILE_PATH_USES_WIN_SEPARATORS)
const FilePath::CharType FilePath::kSeparators[] = ("\\/");
#else   // FILE_PATH_USES_WIN_SEPARATORS
const FilePath::CharType FilePath::kSeparators[] = ("/");
#endif  // FILE_PATH_USES_WIN_SEPARATORS

const size_t FilePath::kSeparatorsLength = arraysize(kSeparators);

const FilePath::CharType FilePath::kCurrentDirectory[] = (".");
const FilePath::CharType FilePath::kParentDirectory[] = ("..");

const FilePath::CharType FilePath::kExtensionSeparator = ('.');

}  // namespace utils
}  // namespace agora
