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

#include "CreateRectangularBranchesTask.h"

#include <QStack>

#include <U2Core/PhyTreeObject.h>
#include <U2Core/U2SafePoints.h>

#include "GraphicsRectangularBranchItem.h"
#include "TreeViewerUtils.h"

namespace U2 {

CreateRectangularBranchesTask::CreateRectangularBranchesTask(const PhyNode *n)
    : size(0), current(0), node(n) {
}

GraphicsRectangularBranchItem *CreateRectangularBranchesTask::getBranch(const PhyNode *node) {
    if (isCanceled() || stateInfo.hasError())
        return nullptr;

    int branches = node->branchCount();
    if (branches == 1 && (node->getName() == "" || node->getName() == "ROOT")) {
        assert(node != node->getSecondNodeOfBranch(0));
        return getBranch(node->getSecondNodeOfBranch(0));
    }
    if (branches > 1) {
        stateInfo.progress = 100 * ++size / 100;  // <- number of sequences
        QList<GraphicsRectangularBranchItem *> items;
        int ind = -1;
        for (int i = 0; i < branches; ++i) {
            if (isCanceled() || stateInfo.hasError()) {
                return nullptr;
            }
            if (node->getSecondNodeOfBranch(i) != node) {
                GraphicsRectangularBranchItem *item = getBranch(node->getSecondNodeOfBranch(i));
                items.append(item);
            } else {
                items.append(nullptr);
                ind = i;
            }
        }

        GraphicsRectangularBranchItem *item = nullptr;
        if (ind < 0) {
            item = new GraphicsRectangularBranchItem();
        } else {
            const PhyBranch *parentBranch = node->getParentBranch();
            if (parentBranch != nullptr) {
                item = new GraphicsRectangularBranchItem(node->getBranchesDistance(ind), node->getBranch(ind), parentBranch->nodeValue);
            }
        }
        SAFE_POINT_EXT(item != nullptr, setError(tr("An internal error: a tree is in an incorrect state, can't create a branch")), nullptr);
        int itemSize = items.size();
        assert(itemSize > 0);

        {
            int xmin = 0, ymin = items[0] ? items[0]->pos().y() : items[1]->pos().y(), ymax = 0;
            for (int i = 0; i < itemSize; ++i) {
                if (items[i] == nullptr) {
                    continue;
                }
                QPointF pos1 = items[i]->pos();
                if (pos1.x() < xmin)
                    xmin = pos1.x();
                if (pos1.y() < ymin)
                    ymin = pos1.y();
                if (pos1.y() > ymax)
                    ymax = pos1.y();
            }
            xmin -= GraphicsRectangularBranchItem::DEFAULT_WIDTH;

            int y = (ymax + ymin) / 2;
            item->setPos(xmin, y);

            for (int i = 0; i < itemSize; ++i) {
                if (items[i] == nullptr) {
                    continue;
                }
                if (isCanceled() || stateInfo.hasError()) {
                    return nullptr;
                }
                qreal dist = qAbs(node->getBranchesDistance(i));
                if (minDistance > -1) {
                    minDistance = qMin(minDistance, dist);
                } else {
                    minDistance = dist;
                }
                maxDistance = qMax(maxDistance, dist);
                items[i]->setDirection(items[i]->pos().y() > y ? GraphicsRectangularBranchItem::up : GraphicsRectangularBranchItem::down);
                items[i]->setWidthW(dist);
                items[i]->setDist(dist);
                items[i]->setParentItem(item);
                QRectF rect = items[i]->getDistanceText()->boundingRect();
                items[i]->getDistanceText()->setPos(-(items[i]->getWidth() + rect.width()) / 2, 0);
            }
        }
        return item;
    } else {
        int y = (current++ + 0.5) * GraphicsRectangularBranchItem::DEFAULT_HEIGHT;
        GraphicsRectangularBranchItem *item = nullptr;
        if (branches != 1) {
            item = new GraphicsRectangularBranchItem(0, y, node->getName());
        } else {
            item = new GraphicsRectangularBranchItem(0, y, node->getName(), node->getBranchesDistance(0), node->getBranch(0));
        }

        return item;
    }
}

void CreateRectangularBranchesTask::run() {
    if (isCanceled() || stateInfo.hasError()) {
        return;
    }
    minDistance = -2;
    maxDistance = 0;
    GraphicsRectangularBranchItem *branchItem = getBranch(node);  // modifies minDistance and maxDistance
    CHECK(branchItem != nullptr, );
    branchItem->setWidthW(0);
    branchItem->setDist(0);
    branchItem->setHeightW(0);
    root = branchItem;

    if (minDistance == 0) {
        minDistance = GraphicsRectangularBranchItem::EPSILON;
    }
    if (maxDistance == 0) {
        maxDistance = GraphicsRectangularBranchItem::EPSILON;
    }
    qreal minDistScale = GraphicsRectangularBranchItem::DEFAULT_WIDTH / (qreal)minDistance;
    qreal maxDistScale = GraphicsRectangularBranchItem::MAXIMUM_WIDTH / (qreal)maxDistance;

    scale = qMin(minDistScale, maxDistScale);

    QStack<GraphicsRectangularBranchItem *> stack;
    stack.push(branchItem);
    while (!stack.empty()) {
        GraphicsRectangularBranchItem *item = stack.pop();
        item->setWidth(item->getWidth() * scale);
        foreach (QGraphicsItem *ci, item->childItems()) {
            GraphicsRectangularBranchItem *gbi = dynamic_cast<GraphicsRectangularBranchItem *>(ci);
            if (gbi != nullptr) {
                stack.push(gbi);
            }
        }
    }
}

}  // namespace U2
