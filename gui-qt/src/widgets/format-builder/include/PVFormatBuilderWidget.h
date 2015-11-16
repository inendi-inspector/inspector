/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVFORMATBUILDER_H
#define	PVFORMATBUILDER_H
#include <iostream>

#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include<QPushButton>
#include<QWidget>
#include<QToolBar>
#include <QFileDialog>
#include <QAction>
#include <QString>
#include <QMenuBar>
#include <QAbstractItemModel>
#include <QMainWindow>
#include <QDomElement>
#include <QDomDocument>
#include <QListWidget>

#include <PVXmlDomModel.h>
#include <PVXmlTreeView.h>
#include <PVXmlParamWidget.h>
#include <PVNrawListingWidget.h>
#include <PVNrawListingModel.h>
#include <pvkernel/core/PVRegistrableClass.h>
#include <pvkernel/core/PVClassLibrary.h>
#include <pvkernel/core/PVArgument.h>
#include <pvkernel/filter/PVFieldsFilterParamWidget.h>
#include <pvkernel/rush/PVRawSourceBase_types.h>
#include <pvkernel/rush/PVSourceCreator.h>
#include <pvkernel/rush/PVExtractor.h>
#include <pvkernel/rush/PVInputType.h>
#include <inendi/PVSource_types.h>

namespace PVGuiQt {
class PVAxesCombinationWidget;
}

namespace PVInspector{

class PVOptionsWidget;

typedef QList<PVFilter::PVFieldsSplitterParamWidget_p> list_splitters_t;
typedef QList<PVFilter::PVFieldsConverterParamWidget_p > list_converters_t;


class PVFormatBuilderWidget : public QMainWindow {
    Q_OBJECT
public:
    PVFormatBuilderWidget(QWidget * parent = NULL);

    virtual ~PVFormatBuilderWidget();

private:
    void closeEvent(QCloseEvent *event);
	void init(QWidget* parent = 0);
	bool somethingChanged(void);

public:
	bool openFormat(QString const& path);
	void openFormat(QDomDocument& doc);
	PVRush::types_groups_t& getGroups() { return myTreeModel->getGroups(); }

private:
    //FIXME: Those variables names are crap!
    PVXmlTreeView *myTreeView;
    PVXmlDomModel *myTreeModel;
    PVXmlParamWidget *myParamBord_old_model;
    QWidget *myParamBord;
    QWidget emptyParamBoard;
    QTabWidget* _main_tab;
    //
    QVBoxLayout *vbParam;
    QMenuBar *menuBar;
	Inendi::PVSource* _org_source; // If this widget is bound to a PVSource's format

    //
    QFile logFile;///!< file we open to edit the format
    int lastSplitterPluginAdding;
    
     QFileDialog _file_dialog;
    
    void actionAllocation();
    
    void hideParamBoard();
    
    /**
     * initialise les connexions dont tout les emitter/reciever sont des attributs
     * de la classe
     */
    void initConnexions();
    
    /**
     * init the menubar
     */
    void initMenuBar();
    
    void setWindowTitleForFile(QString const& path);

    bool save();
    bool saveAs();


    /**
     * init the toolsbar
     * @param vb
     */
    void initToolBar(QVBoxLayout *vb);
    QAction *actionAddAxisAfter;
    QAction *actionAddAxisIn;
    QAction *actionAddFilterAfter;
    QAction *actionAddRegExAfter;
    QAction *actionAddRegExBefore;
    QAction *actionAddUrl;
    QAction *actionAddRegExIn;
    QPushButton *actionApply;
    QAction *actionCloseWindow;
    QAction *actionDelete;
    QAction *actionMoveUp;
    QAction *actionMoveDown;
    QAction *actionNewWindow;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSaveAs;
    
    /**
     * init the splitters list, by listing the plugins found
     */
    void initSplitters();    
    list_splitters_t _list_splitters;///!<list of the plugins splitters
    list_converters_t _list_converters;///!<list of the plugins converters
    
    void showParamBoard(PVRush::PVXmlTreeNodeDom *node);



// Log input management

protected:
	void update_table(PVRow start, PVRow end);
	void set_format_from_dom();
	void create_extractor();
	void guess_first_splitter();
	bool is_dom_empty();

protected:
	PVRush::PVInputDescription_p _log_input;
	PVRush::PVInputType_p _log_input_type;
	PVRush::PVSourceCreator_p _log_sc;
	PVRush::PVRawSourceBase_p _log_source;
	std::shared_ptr<PVRush::PVExtractor> _log_extract;
	PVOptionsWidget* _options_widget;
	PVGuiQt::PVAxesCombinationWidget* _axes_comb_widget;

	// Model and widget for the NRAW
	PVNrawListingModel* _nraw_model;
	PVNrawListingWidget* _nraw_widget;

	// Invalid lines
	QListWidget* _inv_lines_widget;

	static QList<QUrl> _original_shortcuts;

protected:
	QString _cur_file;

private:
	void load_log(PVRow rstart, PVRow rend);

public slots:
	// Tree slots
	void slotAddAxisIn();
	void slotAddFilterAfter();
	void slotAddRegExAfter();
	void slotAddSplitter();
	void slotAddConverter();
	void slotAddUrl();
	void slotApplyModification();
	void slotDelete();
	void slotMoveUp();
	void slotMoveDown();
	void slotNeedApply();
	void slotNewWindow();
	QString slotOpen();
	void slotOpenLog();
	void slotSave();
	void slotSaveAs();
	void slotUpdateToolDesabled(const QModelIndex &);
	void slotExtractorPreview();
	void slotItemClickedInView(const QModelIndex &index);
	void slotMainTabChanged(int idx);

	// Slot for the NRAW listing
	void set_axes_name_selected_row_Slot(int row);
	void set_axes_type_selected_row_Slot(int row);

private:
	PVRush::PVInputType_p            _in_t;
	PVRush::PVInputType::list_inputs _inputs;
};

}
#endif	/* PVFORMATBUILDER_H */

