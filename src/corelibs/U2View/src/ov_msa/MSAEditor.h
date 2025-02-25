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

#ifndef _U2_MSA_EDITOR_H_
#define _U2_MSA_EDITOR_H_

#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/U2Msa.h>

#include "MaEditor.h"
#include "MsaEditorWgt.h"
#include "phy_tree/MSAEditorTreeManager.h"

namespace U2 {

class PairwiseAlignmentTask;
class U2SequenceObject;

class PairwiseAlignmentWidgetsSettings {
public:
    PairwiseAlignmentWidgetsSettings()
        : firstSequenceId(U2MsaRow::INVALID_ROW_ID),
          secondSequenceId(U2MsaRow::INVALID_ROW_ID), inNewWindow(true),
          showSequenceWidget(true), showAlgorithmWidget(false),
          showOutputWidget(false), sequenceSelectionModeOn(false) {
    }

    qint64 firstSequenceId;
    qint64 secondSequenceId;
    QString algorithmName;
    bool inNewWindow;
    QString resultFileName;
    QPointer<PairwiseAlignmentTask> pairwiseAlignmentTask;
    bool showSequenceWidget;
    bool showAlgorithmWidget;
    bool showOutputWidget;
    bool sequenceSelectionModeOn;

    QVariantMap customSettings;
};

/** Variants of the group sort modes supported by UGENE. */
enum class GroupsSortOrder {
    /** Groups are not sorted at all. This is default behaviour of UGENE. */
    Original,

    /** Small groups go first in the name list. */
    Ascending,

    /** Large groups go first in the name list. */
    Descending,
};

class U2VIEW_EXPORT MSAEditor : public MaEditor {
    Q_OBJECT
    Q_DISABLE_COPY(MSAEditor)

    friend class MSAEditorTreeViewerUI;
    friend class SequenceAreaRenderer;
    friend class SequenceWithChromatogramAreaRenderer;

public:
    MSAEditor(const QString &viewName, MultipleSequenceAlignmentObject *obj);
    ~MSAEditor();

    QString getSettingsRoot() const override {
        return MSAE_SETTINGS_ROOT;
    }

    MultipleSequenceAlignmentObject *getMaObject() const override {
        return qobject_cast<MultipleSequenceAlignmentObject *>(maObject);
    }

    /** Returns selection controller instance. The instance is always defined and is never null. */
    MaEditorSelectionController *getSelectionController() const override;

    void buildStaticToolbar(QToolBar *tb) override;

    void buildMenu(QMenu *m, const QString &type) override;

    MsaEditorWgt *getUI() const override;

    // Return alignment row that is displayed on target line in MSAEditor
    MultipleSequenceAlignmentRow getRowByViewRowIndex(int viewRowIndex) const;

    PairwiseAlignmentWidgetsSettings *getPairwiseAlignmentWidgetsSettings() const {
        return pairwiseAlignmentWidgetsSettings;
    }

    MSAEditorTreeManager *getTreeManager() {
        return &treeManager;
    }

    void buildTree();

    QString getReferenceRowName() const override;

    char getReferenceCharAt(int pos) const override;

    void sortSequences(const MultipleAlignment::SortType &sortType, const MultipleAlignment::Order &sortOrder);

    /** Forces complete re-computation of the active collapse model based on the current MSA editor state. */
    void updateCollapseModel();

    void setRowOrderMode(MaEditorRowOrderMode mode) override;

    /** Returns current set of free-mode markers. */
    const QSet<QObject *> &getFreeModeMasterMarkersSet() const;

    /** Adds new marker object into freeModeMasterMarkersSet. */
    void addFreeModeMasterMarker(QObject *marker);

    /** Removes the given marker object from the freeModeMasterMarkersSet. */
    void removeFreeModeMasterMarker(QObject *marker);

protected slots:
    void sl_onContextMenuRequested(const QPoint &pos) override;

    void sl_buildTree();
    void sl_align();
    void sl_addToAlignment();
    void sl_searchInSequences();
    void sl_searchInSequenceNames();
    void sl_realignSomeSequences();
    void sl_setSeqAsReference();
    void sl_unsetReferenceSeq();

    void sl_showTreeOP();
    void sl_hideTreeOP();
    void sl_rowsRemoved(const QList<qint64> &rowIds);
    void sl_updateRealignAction();
    void sl_showCustomSettings();
    void sl_sortSequencesByName();
    void sl_sortSequencesByLength();
    void sl_sortSequencesByLeadingGap();

    /**
     * Slot for sortByLeadingGap(Ascending/Descending)Action.
     * Re-sorts group of sequences based on the sender action: using ascending or descending order.
     */
    void sl_sortGroupsBySize();

    /** Converts from DNA to RNA alphabet and back. */
    void sl_convertBetweenDnaAndRnaAlphabets();

    /** Converts from RAW to DNA alphabet. Replaces all unknown chars with 'N' and 'U' with 'T'. */
    void sl_convertRawToDnaAlphabet();

    /** Converts from RAW to Amino alphabet. Replaces all unknown chars with 'X'. */
    void sl_convertRawToAminoAlphabet();

    /** Shows 'Export Image' dialog. */
    void sl_exportImage();

protected:
    QWidget *createWidget() override;
    void initActions() override;
    bool eventFilter(QObject *o, QEvent *e) override;
    bool onObjectRemoved(GObject *obj) override;
    void onObjectRenamed(GObject *obj, const QString &oldName) override;
    bool onCloseEvent() override;

    void addCopyPasteMenu(QMenu *m) override;
    void addEditMenu(QMenu *m) override;
    void addSortMenu(QMenu *m);
    void addExportMenu(QMenu *m) override;
    void addAppearanceMenu(QMenu *m);
    void addColorsMenu(QMenu *m);
    void addHighlightingMenu(QMenu *m);
    void addNavigationMenu(QMenu *m);
    void addTreeMenu(QMenu *m);
    void addAdvancedMenu(QMenu *m);
    void addStatisticsMenu(QMenu *m);

    void updateActions() override;

    void initDragAndDropSupport();

public:
    QAction *buildTreeAction = nullptr;
    QAction *alignAction = nullptr;
    QAction *alignSequencesToAlignmentAction = nullptr;
    QAction *realignSomeSequenceAction = nullptr;
    QAction *setAsReferenceSequenceAction = nullptr;
    QAction *unsetReferenceSequenceAction = nullptr;
    QAction *gotoAction = nullptr;
    QAction *searchInSequencesAction = nullptr;
    QAction *searchInSequenceNamesAction = nullptr;
    QAction *openCustomSettingsAction = nullptr;
    QAction *sortByNameAscendingAction = nullptr;
    QAction *sortByNameDescendingAction = nullptr;
    QAction *sortByLengthAscendingAction = nullptr;
    QAction *sortByLengthDescendingAction = nullptr;
    QAction *sortByLeadingGapAscendingAction = nullptr;
    QAction *sortByLeadingGapDescendingAction = nullptr;

    /** Initiates a dialog to export the alignment using some image format. */
    QAction *saveScreenshotAction = nullptr;

    /**
     * Sorts collapsing groups by number of sequences in ascending order.
     * The action is only enabled in 'MaEditorRowOrderMode::Sequence' mode when there are groups of length >=2.
     */
    QAction *sortGroupsBySizeAscendingAction = nullptr;

    /**
     * Sorts collapsing groups by number of sequences in descending descending order.
     * The action is only enabled in 'MaEditorRowOrderMode::Sequence' mode when there are groups of length >=2.
     */
    QAction *sortGroupsBySizeDescendingAction = nullptr;

    QAction *convertDnaToRnaAction = nullptr;
    QAction *convertRnaToDnaAction = nullptr;
    QAction *convertRawToDnaAction = nullptr;
    QAction *convertRawToAminoAction = nullptr;

private:
    PairwiseAlignmentWidgetsSettings *pairwiseAlignmentWidgetsSettings = nullptr;
    MSAEditorTreeManager treeManager;

    /**
     * Sort order for groups.
     * Default is 'Descending' - groups with the most sequences are on top.
     */
    GroupsSortOrder groupsSortOrder = GroupsSortOrder::Original;

    /**
     * Set of 'marker' objects from the 'master' components that requested Free ordering mode to be ON are responsible for the 'free' mode ordering.
     * Free mode can be active only if there is at least one 'marker' in the set.
     *
     * When the last marker object is removed from the set the ordering automatically switches to the 'Original'.
     * Example of master components: multiple synchronized phy-tree views that manage the order of MSA.
     *
     * MSAEditor can any time reset this set and switch to 'Original' or 'Sequence' mode.
     */
    QSet<QObject *> freeModeMasterMarkersSet;

    /** Selection state controller. */
    MaEditorSelectionController *selectionController;
};

/** Set of custom menu actions in MSA editor. */
class U2VIEW_EXPORT MsaEditorMenuType : public GObjectViewMenuType {
public:
    /** "Align" button menu identifier. */
    const static QString ALIGN;

    /** "Align sequence(s) to this alignment" menu identifier. */
    const static QString ALIGN_SEQUENCES_TO_ALIGNMENT;
};

}  // namespace U2

#endif  // _U2_MSA_EDITOR_H_
