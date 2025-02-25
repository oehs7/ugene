/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2021 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _ADD_FOLDER_DIALOG_FILLER_H_
#define _ADD_FOLDER_DIALOG_FILLER_H_

#include <base_dialogs/GTFileDialog.h>

#include "utils/GTUtilsDialog.h"

namespace U2 {
using namespace HI;

class AddFolderDialogFiller : public Filler {
public:
    AddFolderDialogFiller(HI::GUITestOpStatus &os, const QString &folderName, GTGlobals::UseMethod acceptMethod);
    void commonScenario();

private:
    const QString folderName;
    const GTGlobals::UseMethod acceptMethod;
};

}  // namespace U2

#endif  // _ADD_FOLDER_DIALOG_FILLER_H_
