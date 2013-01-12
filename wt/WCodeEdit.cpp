/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * WCodeEdit Copyright (c) 2012-2013 Michael Wiklund
 * Using WTextEdit as a template.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "WCodeEdit.h"
#include "Wt/WBoostAny"

#include "DomElement.h"

namespace Wt {

typedef std::map<std::string, boost::any> SettingsMapType;

WCodeEdit::WCodeEdit(WContainerWidget *parent)
  : WTextArea(parent),
    contentChanged_(false)
{
  init();
}

WCodeEdit::WCodeEdit(const WT_USTRING& text, WContainerWidget *parent)
  : WTextArea(text, parent),
    contentChanged_(false)
{
  init();
}

void WCodeEdit::init()
{
  // WApplication *app = WApplication::instance();

  setInline(false);

  initCodeMirror();
  
  setJavaScriptMember
    (WT_RESIZE_JS,
     "function(e,w,h){e.ed.setSize(w-2, h); e.ed.refresh();};");
  /*
  setConfigurationSetting("button_tile_map", true);
*/
}

WCodeEdit::~WCodeEdit()
{
  // to have virtual renderRemoveJs():
  setParentWidget(0);
}

void WCodeEdit::setStyleSheet(const std::string& /*uri*/)
{
  //setConfigurationSetting("content_css", uri);
}

const std::string WCodeEdit::styleSheet() const
{
  return "";
  // return asString(configurationSetting("content_css")).toUTF8();
}

/*
void WCodeEdit::setExtraPlugins(const std::string& plugins)
{
  setConfigurationSetting("plugins", plugins);
}

const std::string WCodeEdit::extraPlugins() const
{
  return asString(configurationSetting("plugins")).toUTF8();
}
*/

std::string WCodeEdit::renderRemoveJs()
{
  if (isRendered())
    return jsRef() + ".ed.toTextArea();" WT_CLASS ".remove('" + id() + "');";
  else
    return WTextArea::renderRemoveJs();
}

void WCodeEdit::initCodeMirror()
{
  std::string codeMirrorBaseURL = WApplication::resourcesUrl() + "code_mirror/";

  WApplication::readConfigurationProperty("codeMirrorBaseURL", codeMirrorBaseURL);

  if (!codeMirrorBaseURL.empty()
      && codeMirrorBaseURL[codeMirrorBaseURL.length()-1] != '/')
    codeMirrorBaseURL += '/';

  WApplication *app = WApplication::instance();

  // if (app->environment().ajax())
  //  app->doJavaScript("window.codeMirror_GZ = { loaded: true };", false);

  if (app->require(codeMirrorBaseURL + "lib/codemirror.js", "window['codeMirror']")) {
    app->useStyleSheet(codeMirrorBaseURL + "lib/codemirror.css");
    if (app->require(codeMirrorBaseURL + "mode/javascript/javascript.js", "window['codeMirror_js']")) {
    }
    /*
      we should not use display:none for hiding?
    */
/*
    app->styleSheet().addRule(".mceEditor", "height: 100%;");

    // Adjust the height: this can only be done by adjusting the iframe height.
    app->doJavaScript
      (WT_CLASS ".tinyMCEResize=function(e,w,h){"
       """e.style.height = (h - 2) + 'px';"
       "};", false);
*/
  }
}

void WCodeEdit::resize(const WLength& width, const WLength& height)
{
  WTextArea::resize(width, height);
}

void WCodeEdit::setText(const WT_USTRING& text)
{
  WTextArea::setText(text);
  contentChanged_ = true;
}

/*
std::string WCodeEdit::plugins() const
{
  std::string plugins = extraPlugins();
  if (!plugins.empty())
    plugins += ",";
  plugins += "safari";
  return plugins;
}
*/

void WCodeEdit::updateDom(DomElement& element, bool all)
{
  WTextArea::updateDom(element, all);

  if (element.type() == DomElement_TEXTAREA)
    element.removeProperty(PropertyStyleDisplay);

  // we are creating the actual element
  if (all && element.type() == DomElement_TEXTAREA) {
    std::stringstream config;
    config << "{";

    bool first = true;

    for (SettingsMapType::const_iterator it = optionSettings_.begin();
     it != optionSettings_.end(); ++it) {
      //if (it->first == "plugins")
      //  continue;

      if (!first)
	config << ',';

      first = false;

      config << it->first << ": "
	     << Impl::asJSLiteral(it->second, XHTMLUnsafeText);
    }
/*
    if (!first)
      config << ',';

    config << "plugins: '" << plugins() << "'";
*/
    config <<
      // ",init_instance_callback: " << jsRef() << ".init" << ""
      "}";

    DomElement dummy(DomElement::ModeUpdate, DomElement_TABLE);
    updateDom(dummy, true);

    /*
     * When initialized, we apply the inline style.
     */
/*
    element.callMethod("init=function(){"
		       "var d=" WT_CLASS ".getElement('" + id() + "_tbl');"
		       "d.style.cssText='width:100%;" + dummy.cssStyle() + "';"
		       "};");
*/
    element.callMethod("ed=CodeMirror.fromTextArea(document.getElementById('" + id() + "'),"
                       + config.str() + ");");

    contentChanged_ = false;
    optionSettingsNew_.clear();
  }

  if (!all && contentChanged_) {
    element.callJavaScript(jsRef() + ".ed.setValue(" + WTextArea::text().jsStringLiteral() + ");");
    contentChanged_ = false;
  }

  if (!all && !optionSettingsNew_.empty()) {
    for (SettingsMapType::const_iterator it = optionSettingsNew_.begin();
     it != optionSettingsNew_.end(); ++it) {
      std::stringstream edCall;
      edCall << jsRef() << ".ed.setOption(" << it->first << ",";
      edCall << Impl::asJSLiteral(it->second, XHTMLUnsafeText) << ");";
      element.callJavaScript(edCall.str());
    }
    optionSettingsNew_.clear();
  }
}

void WCodeEdit::getDomChanges(std::vector<DomElement *>& result,
			      WApplication *app)
{
  /*
   * We apply changes directly to the table element, except of the textarea
   * contents. Therefore we first update the TABLE element, then collect
   * the contentChange to the TEXTAREA element, but reverse the order in
   * which they get applied since the load() statement expects the contents
   * to be set in the textarea first.
   */

  /*
   * Problem! ed.render() returns before the element is actually rendered,
   * and therefore, the _tbl element may not yet be available.
   *
   * This causes fail when a text edit is progressively enhanced. The solution
   * is to listen for the onInit() event -> we should be able to add a
   * wrapping ... .onInit(function(ed) { .... }) around the changes
   */
  // DomElement *e = DomElement::getForUpdate(formName() + "_tbl",
  // 					   DomElement_TABLE);
  // updateDom(*e, false);

  WTextArea::getDomChanges(result, app);

  // result.push_back(e);
}

bool WCodeEdit::domCanBeSaved() const
{
  return false;
}

int WCodeEdit::boxPadding(Orientation /*orientation*/) const
{
  return 0;
}

int WCodeEdit::boxBorder(Orientation /*orientation*/) const
{
  return 1;
}

void WCodeEdit::setOptionSetting(const std::string& name,
					const boost::any& value)
{
    optionSettings_[name] = value;
    optionSettingsNew_[name] = value;
}

boost::any WCodeEdit::optionSetting(const std::string& name) const
{
  SettingsMapType::const_iterator it = optionSettings_.find(name);

  if (it != optionSettings_.end())
    return it->second;
  else
    return boost::any();
}

}
