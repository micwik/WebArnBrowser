// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * WCodeEdit Copyright (c) 2012-2013 Michael Wiklund
 * Using WTextEdit as a template.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCODEEDIT_H_
#define WCODEEDIT_H_

#include <Wt/WTextArea>

#include <boost/any.hpp>

namespace Wt {

/*! \class WTextEdit Wt/WTextEdit Wt/WTextEdit
 *  \brief A rich-text XHTML editor.
 *
 * The editor provides interactive editing of XHTML text. By default
 * it provides basic mark-up (font, formatting, color, links, and
 * lists), but additional buttons may be added to the tool bars that
 * add additional formatting options.
 *
 * The implementation is based on <a
 * href="http://tinymce.moxiecode.com/">TinyMCE</a>. The widget may be
 * configured and tailored using the setConfigurationSetting() and
 * related methods that provide direct access to the underlying
 * TinyMCE component.
 * 
 * \if cpp
 * To use this widget, you need to download TinyMCE (version 3.5b1 or
 * later) and deploy the <tt>tinymce/jscripts/tiny_mce</tt> folder to
 * <i>tinyMCEBaseURL</i>. The default value for <i>tinyMCEBaseURL</i> is
 * <i>resourcesURL</i><tt>/tiny_mce</tt>, where <i>resourcesURL</i> is
 * the configuration property that locates the %Wt <tt>resources/</tt>
 * folder (i.e., we assume by default that you copy the
 * <tt>tiny_mce</tt> folder to the <tt>resources/</tt> folder), see
 * also \ref deployment "deployment and resources".
 *
 * The value may be overridden with a URL that points to the directory
 * where the <tt>tiny_mce</tt> folder is located, by configuring the
 * <i<>tinyMCEBaseURL/i> property in your %Wt configuration file, see \ref
 * config_general "configuration properties".
 * \endif
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WContainerWidget *w = new Wt::WContainerWidget();
 * Wt::WLabel *label = new Wt::WLabel("Comments:", w);
 * Wt::WTextEdit *edit = new Wt::WTextEdit("", w);
 * label->setBuddy(edit);
 * \endcode
 * \endif
 *
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 *
 * \image html WTextEdit-1.png "Default configuration of a WTextEdit"
 */
class WT_API WCodeEdit : public WTextArea
{
public:
  /*! \brief Creates a new text editor.
   */
  WCodeEdit(WContainerWidget *parent = 0);

  /*! \brief Creates a new text editor and initialize with given text.
   *
   * The \p text should be valid XHTML.
   */
  WCodeEdit(const WT_USTRING& text, WContainerWidget *parent = 0);

  /*! \brief Destructor.
   */
  ~WCodeEdit();

  /*! \brief Sets the content.
   *
   * The \p text should be valid XHTML.
   *
   * The default value is "".
   */
  virtual void setText(const WT_USTRING& text);

  /*! \brief Sets the stylesheet for displaying the content.
   *
   * The content is rendered using the rules defined in this
   * stylesheet. The stylesheet is also used to derive additional
   * styles that are available in the text editor, for example in the
   * "styleselect" button.
   *
   * Multiple stylesheets may be specified as a comma separated list.
   */
  void setStyleSheet(const std::string& uri);

  /*! \brief Returns the content stylesheet.
   *
   * \sa setStyleSheet()
   */
  const std::string styleSheet() const;

  /*! \brief Loads additional TinyMCE plugins.
   *
   * %Wt loads by default only the plugin 'safari' (which adds support
   * for the Safari web browser). Use this method to load additional
   * plugins. Multiple plugins may be specified as a comma separated
   * list.
   *
   * The various plugins are described in the <a
   * href="http://wiki.moxiecode.com/index.php/TinyMCE:Plugins">TinyMCE
   * documentation</a>.
   *
   * \note Plugins can only be loaded before the initial display of
   * the widget.
   */
  //void setExtraPlugins(const std::string& plugins);

  /*! \brief Returns the extra plugins.
   *
   * \sa setExtraPlugins()
   */
  //const std::string extraPlugins() const;

  /*! \brief Configures a tool bar.
   *
   * This configures the buttons for the \p i'th tool bar (with 0
   * <= \p i <= 3).
   *
   * The syntax and available buttons is documented in the <a
   * href="http://wiki.moxiecode.com/index.php/TinyMCE:Configuration/theme_advanced_buttons_1_n">TinyMCE
   * documentation</a>.
   *
   * The default <i>config</i> for the first tool bar (\p i = 0)
    is: "fontselect, |, bold, italic, underline, |, fontsizeselect, |,
    forecolor, backcolor, |, justifyleft, justifycenter, justifyright,
    justifyfull, |, anchor, |, numlist, bullist".
   *
   * By default, the other three tool bars are disabled (\p config = "").
   *
   * Some buttons are only available after loading extra plugins using
   * setExtraPlugins().
   *
   * \note The tool bar configuration can only be set before the
   * initial display of the widget.
   */
  //void setToolBar(int i, const std::string& config);

  /*! \brief Returns a tool bar configuration.
   *
   * \sa setToolBar()
   */
  //const std::string toolBar(int i) const;

  /*! \brief Configure a TinyMCE setting.
   *
   * A list of possible settings can be found at:
   * http://tinymce.moxiecode.com/wiki.php/Configuration.
   *
   * The widget itself will also define a number of configuration settings
   * and these may be overridden using this method.
   */
  void setOptionSetting(const std::string& name,
			       const boost::any& value);

  /*! \brief Returns a TinyMCE configuration setting's value.
   *
   * An empty boost::any is returned when no value could be found for the 
   * provided argument.
   */
  boost::any optionSetting(const std::string& name) const;

  virtual void resize(const WLength& width, const WLength& height);

protected:
  virtual std::string renderRemoveJs();
  virtual void updateDom(DomElement& element, bool all);
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app);
  virtual bool domCanBeSaved() const;

  virtual int boxPadding(Orientation orientation) const;
  virtual int boxBorder(Orientation orientation) const;

private:
  bool contentChanged_;
  std::map<std::string, boost::any> optionSettings_;
  std::map<std::string, boost::any> optionSettingsNew_;

  std::string plugins() const;

  void init();
  static void initCodeMirror();
};

}

#endif // WCODEEDIT_H_
