/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <PVXmlTreeView.h>
#include <PVXmlDomModel.h>
#include<QMouseEvent>
#include<QTreeView>
#include<QTreeWidgetItem>
#include<QRectF>
#include <QAbstractItemView>
#include <QStyleOptionViewItem>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
#include <QString>
#include<QRect>
#include<QScrollBar>
#include<qglobal.h>
#include<QDragMoveEvent>

#define dbg()  {qDebug()<<__FILE__<<__LINE__;}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::PVXmlTreeView
 *
 *****************************************************************************/
PVInspector::PVXmlTreeView::PVXmlTreeView(QWidget * parent ) :QTreeView(parent) //
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    isDraging = false;
    isEditing = false;
    setObjectName("PVXmlTreeView");
}




/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::~PVXmlTreeView
 *
 *****************************************************************************/
PVInspector::PVXmlTreeView::~PVXmlTreeView() {
}




/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::addAxisIn
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::addAxisIn() {
  addNode(addAxis);
}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::addFilterAfter
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::addFilterAfter() {
  addNode(addFilter);
}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::addSplitter
 *
 *****************************************************************************/
PVRush::PVXmlTreeNodeDom* PVInspector::PVXmlTreeView::addSplitter(PVFilter::PVFieldsSplitterParamWidget_p splitterPlugin){
        PVLOG_DEBUG("PVInspector::PVXmlTreeView::addSplitter \n");
        QModelIndex index = getSelectedIndex();
        return getModel()->addSplitter(index,splitterPlugin);
}

/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::addConverter
 *
 *****************************************************************************/
PVRush::PVXmlTreeNodeDom* PVInspector::PVXmlTreeView::addConverter(PVFilter::PVFieldsConverterParamWidget_p converterPlugin){
	PVLOG_DEBUG("PVInspector::PVXmlTreeView::addConverter \n");
	QModelIndex index = getSelectedIndex();
	return getModel()->addConverter(index,converterPlugin);
}


/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::addRegExIn
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::addRegExIn() {
  addNode(addRegEx);
}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::addUrlIn
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::addUrlIn() {
  addNode(addUrl);
}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::addNode
 *
 *****************************************************************************/
/**
* add a new node (axis, filter, regexp or url)
* @param type
*/
void PVInspector::PVXmlTreeView::addNode(AddType type){

	QModelIndex index = getSelectedIndex();
	QModelIndex indexToSelect;
	
        int numberOfSelectedIndexes = selectedIndexes().count();
	
	switch (type) {
		//if we want to add a regexp
		case addRegEx:
			getModel()->addRegExIn(index);
			indexToSelect = index.child(getModel()->nodeFromIndex(index)->countChildren() - 1, 0);//last child of the node.
			break;

		//if we want to add a filter
		case addFilter:
			getModel()->addFilterAfter(index);
			indexToSelect = index.child(0, 0);//the first child
			break;

		//if we want to add an axis
		case addAxis:
			getModel()->addAxisIn(index);//add the element
			if ( numberOfSelectedIndexes > 0) {
				indexToSelect = index.child(0, 0).child(0,0);//set the index to select after the adding.
			}
			break;

		//if we want to add a splitter url
		case addUrl:
			getModel()->addUrlIn(index);//add the element
			indexToSelect = index.child(getModel()->nodeFromIndex(index)->countChildren() - 1, 0);//last child of the node.
			expandAll();
		break;
		
		default:
			break;
	}
	
	if (index.isValid()) {
		expandRecursive(index);
	}
	
	//selection of the item just after the creation
	if (indexToSelect.isValid()) {
		selectionModel()->select(indexToSelect, QItemSelectionModel::ClearAndSelect);
		emit clicked(indexToSelect);//the new item become the new selected item.
	}
}




/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::applyModification
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::applyModification(PVXmlParamWidget *paramBord,QModelIndex& ){
  //if(selectedIndexes())PVLOG_ERROR("selectedIndexes() is null in PVInspector::PVXmlTreeView::applyModification(PVXmlParamWidget *paramBord)\n");
	if (paramBord==NULL) {
		PVLOG_ERROR("paramBord is null in PVInspector::PVXmlTreeView::applyModification(PVXmlParamWidget *paramBord)\n");
	}
	
	if (getModel()==NULL) {
		PVLOG_ERROR("getModel() is null in PVInspector::PVXmlTreeView::applyModification(PVXmlParamWidget *paramBord)\n");
	}
  

	if (selectedIndexes().count()>0) {
		//if an item is selected
		QModelIndex index = selectedIndexes().at(0);
		if (!index.isValid()) {
			PVLOG_ERROR("index invalid in PVInspector::PVXmlTreeView::applyModification(PVXmlParamWidget *paramBord)\n");
		}
		getModel()->applyModification(index,paramBord); 
		if (index.isValid()) {
			expandRecursive(index);
		}
	}

}




/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::deleteSelection
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::deleteSelection()
{
	QModelIndexList sels_idx = selectedIndexes();
	foreach(QModelIndex const& index, sels_idx) {
		if (index.column() != 0) {
			continue;
		}
        QModelIndex parentIndex = index.parent();
        if (parentIndex.isValid()) {
            collapse(parentIndex);
            expandRecursive(parentIndex);
            if (parentIndex.isValid()) {
				emit clicked(parentIndex);
			}
		}
		getModel()->deleteSelection(index);
    }
}




/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::expandRecursive
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::expandRecursive(const QModelIndex &index){
    expand(index);

    //expand for all child
    for(int i=0;i<getModel()->rowCount(index);i++) {
        expandRecursive(index.child(i, 0));
    }

    //update size of the first column
    //calculate column size needed.
    if (index.isValid()) {
        int l_width = itemDelegate(index)->sizeHint(QStyleOptionViewItem(), index).width();
        int l_offset = getModel()->countParent(index)*20;

        //resize column
        if (columnWidth(0) < l_width + l_offset+30) {
            setColumnWidth(0, l_width + l_offset+30);
        }
    }
}




/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::getModel
 *
 *****************************************************************************/
PVInspector::PVXmlDomModel* PVInspector::PVXmlTreeView::getModel(){
  if(model()==NULL){
    PVLOG_ERROR("no model in PVInspector::PVXmlTreeView::getModel()\n");
    return NULL;
  }
  return ((PVXmlDomModel*) model());
}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::mouseDoubleClickEvent
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::mouseDoubleClickEvent ( QMouseEvent * event ){
        QTreeView::mouseDoubleClickEvent(event);
        isEditing = true;
}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::mousePressEvent
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::mousePressEvent(QMouseEvent * event)
{
	QTreeView::mousePressEvent(event);
	
	// AG: i just can't figure out the interest of this...
	// It seemed to be the cause of bug #119
#if 0
	PVLOG_DEBUG("PVInspector::PVXmlTreeView::mousePressEvent\n");
	for (int i = 0; i < selectedIndexes().count(); i++) {
		if (selectedIndexes().at(i).isValid()){
			selectionModel()->select(selectedIndexes().at(i), QItemSelectionModel::Clear); //valid index...
		}
	}

	QTreeView::mousePressEvent(event);
	//QTreeView::
	QModelIndex index;
	if (index.isValid()) {
		PVLOG_DEBUG("emit clicked() on an invalid index in PVInspector::PVXmlTreeView::mousePressEvent()\n");
	}
	emit clicked(index); //keep it anyway, if the index is not valid, it update toolsbar enabled tools.
#endif
}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::moveDown
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::moveDown() {
    PVLOG_DEBUG("PVInspector::PVXmlTreeView::moveDown\n");
    if (selectedIndexes().count() > 0) {//if an item is selected
        QModelIndex index = selectedIndexes().at(0);
        getModel()->moveDown(index);
        if (index.row() < getModel()->nodeFromIndex(index)->getParent()->countChildren()-1)//if child isn't the last
            if(index.parent().child(index.row() + 1, 0).isValid()){//valid index...
	      selectionModel()->select(index.parent().child(index.row() + 1, 0), QItemSelectionModel::ClearAndSelect);
	      emit clicked(index.parent().child(index.row() + 1, 0));
	    }
    }
}



/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::moveUp
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::moveUp() {
    if (selectedIndexes().count() > 0) {//if an item is selected
        QModelIndex index = selectedIndexes().at(0);
        getModel()->moveUp(index);
        if (index.row() > 0){//if child isn't the first
	  if(index.parent().child(index.row() - 1, 0).isValid()){//valid index...
            selectionModel()->select(index.parent().child(index.row() - 1, 0), QItemSelectionModel::ClearAndSelect);
	    emit clicked(index.parent().child(index.row() - 1, 0));
	  }
	}
    }
}

/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::slotDataHasChanged
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::slotDataHasChanged(const QModelIndex & index, const QModelIndex & ){
        PVLOG_DEBUG("PVInspector::PVXmlTreeView::slotDataHasChanged\n");
        emit clicked(index);
}

/******************************************************************************
 *
 * PVInspector::PVXmlTreeView::slotSelectNext
 *
 *****************************************************************************/
void PVInspector::PVXmlTreeView::slotSelectNext(){
  if (selectedIndexes().count() > 0) {//if an item is selected...
    QModelIndex index = selectedIndexes().at(0);
    if(index.isValid()){//if the index is valid...
      if(index.parent().isValid()){
	QModelIndex selectIndex =getModel()->selectNext(index);
	if(selectIndex.isValid()){//if the new index is valid
	    
            if(isEditing){
                    PVLOG_DEBUG("has EditFocus\n");
                    isEditing = false;
                    selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
                    emit clicked(index);
            }else{
                    PVLOG_DEBUG("hasn't EditFocus\n");
                    selectionModel()->select(selectIndex, QItemSelectionModel::ClearAndSelect);
                    emit clicked(selectIndex);
            }
	}
      }
    }
  }
}

