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

#include "MsaEditorNameList.h"

#include <QMouseEvent>

#include <U2Gui/GUIUtils.h>

#include "MSAEditor.h"

namespace U2 {

MsaEditorNameList::MsaEditorNameList(MaEditorWgt *ui, QScrollBar *nhBar)
    : MaEditorNameList(ui, nhBar) {
    connect(editor, SIGNAL(si_buildMenu(GObjectView *, QMenu *, const QString &)), SLOT(sl_buildMenu(GObjectView *, QMenu *, const QString &)));
}

void MsaEditorNameList::sl_buildMenu(GObjectView *, QMenu *menu, const QString &menuType) {
    if (menuType == MsaEditorMenuType::CONTEXT || menuType == MsaEditorMenuType::STATIC) {
        buildMenu(menu);
    }
}

void MsaEditorNameList::buildMenu(QMenu *menu) {
    QMenu *editMenu = GUIUtils::findSubMenu(menu, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != nullptr, "editMenu not found", );

    editMenu->insertAction(editMenu->actions().last(), removeSequenceAction);

    CHECK(qobject_cast<MSAEditor *>(editor) != nullptr, );
    CHECK(rect().contains(mapFromGlobal(QCursor::pos())), );

    editMenu->insertAction(editMenu->actions().first(), editSequenceNameAction);
}

MSAEditor *MsaEditorNameList::getEditor() const {
    return qobject_cast<MSAEditor *>(editor);
}

void MsaEditorNameList::mouseDoubleClickEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        sl_editSequenceName();
        e->ignore();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}

}  // namespace U2
