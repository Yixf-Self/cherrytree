﻿/*
 * ct_actions_export.cc
 *
 * Copyright 2017-2020 Giuseppe Penone <giuspen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "ct_actions.h"
#include "ct_export2pdf.h"
#include "ct_export2html.h"
#include "ct_export2txt.h"
#include "ct_storage_control.h"
#include <glib/gstdio.h>
#include "ct_logging.h"

// Print Page Setup Operations
void CtActions::export_print_page_setup()
{
    _pCtMainWin->get_ct_print().run_page_setup_dialog(_pCtMainWin);
}

void CtActions::export_print()
{
    _export_print(false, "", false);
}

void CtActions::export_to_pdf()
{
    _export_print(true, "", false);
}

void CtActions::export_to_html()
{
    _export_to_html("", false);
}

void CtActions::export_to_txt_multiple()
{
    _export_to_txt(false, "", false);
}

void CtActions::export_to_txt_single()
{
    _export_to_txt(true, "", false);
}

void CtActions::export_to_ctd()
{

}

void CtActions::export_to_pdf_auto(const std::string& dir, bool overwrite)
{
    spdlog::debug("pdf export to: {}", dir);
    spdlog::debug("overwrite: {}", overwrite);
    _export_print(true, dir, overwrite);
}

void CtActions::export_to_html_auto(const std::string& dir, bool overwrite)
{
    spdlog::debug("html export to: {}", dir);
    spdlog::debug("overwrite: {}", overwrite);
    _export_to_html(dir, overwrite);
}

void CtActions::export_to_txt_auto(const std::string& dir, bool overwrite)
{
    spdlog::debug("txt export to: {}", dir);
    spdlog::debug("overwrite: {}", overwrite);
    _export_to_txt(false, dir, overwrite);
}


void CtActions::_export_print(bool save_to_pdf, Glib::ustring auto_path, bool auto_overwrite)
{
    if (!_is_there_selected_node_or_error()) return;
    auto export_type = auto_path != "" ? CtDialogs::CtProcessNode::ALL_TREE
                                       : CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_export_options.include_node_name,
                                                                                         &_export_options.new_node_page, nullptr);
    if (export_type == CtDialogs::CtProcessNode::NONE) return;

    Glib::ustring pdf_filepath;
    if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE)
    {
        if (save_to_pdf)
        {
            pdf_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
            pdf_filepath = _get_pdf_filepath(pdf_filepath);
            if (pdf_filepath == "") return;
        }
        CtExport2Pdf(_pCtMainWin).node_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options, -1, -1);
    }
    else if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE_AND_SUBNODES)
    {
        if (save_to_pdf)
        {
            pdf_filepath = _get_pdf_filepath(_pCtMainWin->get_ct_storage()->get_file_name());
            if (pdf_filepath == "") return;
        }
        CtExport2Pdf(_pCtMainWin).node_and_subnodes_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options);
    }
    else if (export_type == CtDialogs::CtProcessNode::ALL_TREE)
    {
        if (auto_path != "")
        {
            pdf_filepath = Glib::build_filename(auto_path, _pCtMainWin->get_ct_storage()->get_file_name() + ".pdf");
            if (!auto_overwrite && Glib::file_test(pdf_filepath, Glib::FILE_TEST_IS_REGULAR)) {
                spdlog::info("pdf exists and overwrite is off, export is stopped"); 
                return;
            }
        }
        else if (save_to_pdf)
        {
            pdf_filepath = _get_pdf_filepath(_pCtMainWin->get_ct_storage()->get_file_name());
            if (pdf_filepath == "") return;
        }
        CtExport2Pdf(_pCtMainWin).tree_export_print(pdf_filepath, _pCtMainWin->get_tree_store().get_ct_iter_first(), _export_options);
    }
    else if (export_type == CtDialogs::CtProcessNode::SELECTED_TEXT)
    {
        if (!_is_there_text_selection_or_error()) return;
        Gtk::TextIter iter_start, iter_end;
        _curr_buffer()->get_selection_bounds(iter_start, iter_end);

        if (save_to_pdf)
        {
            pdf_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
            pdf_filepath = _get_pdf_filepath(pdf_filepath);
            if (pdf_filepath == "") return;
        }
        CtExport2Pdf(_pCtMainWin).node_export_print(pdf_filepath, _pCtMainWin->curr_tree_iter(), _export_options, iter_start.get_offset(), iter_end.get_offset());
    }
}

// Export to HTML
void CtActions::_export_to_html(Glib::ustring auto_path, bool auto_overwrite)
{
    if (!_is_there_selected_node_or_error()) return;
    auto export_type = auto_path != "" ? CtDialogs::CtProcessNode::ALL_TREE
                                       : CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_export_options.include_node_name,
                                                                                         nullptr, &_export_options.index_in_page);
    if (export_type == CtDialogs::CtProcessNode::NONE) return;

    CtExport2Html export2html(_pCtMainWin);
    Glib::ustring ret_html_path;
    if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE)
    {
        Glib::ustring folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false, ret_html_path))
            export2html.node_export_to_html(_pCtMainWin->curr_tree_iter(), _export_options, "", -1, -1);
    }
    else if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE_AND_SUBNODES)
    {
        Glib::ustring folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false, ret_html_path))
            export2html.nodes_all_export_to_html(false, _export_options);
    }
    else if (export_type == CtDialogs::CtProcessNode::ALL_TREE)
    {
        Glib::ustring folder_name = _pCtMainWin->get_ct_storage()->get_file_name();
        if (export2html.prepare_html_folder(auto_path, folder_name, auto_overwrite, ret_html_path))
            export2html.nodes_all_export_to_html(true, _export_options);
    }
    else if (export_type == CtDialogs::CtProcessNode::SELECTED_TEXT)
    {
        if (!_is_there_text_selection_or_error()) return;
        Gtk::TextIter iter_start, iter_end;
        _curr_buffer()->get_selection_bounds(iter_start, iter_end);

        Glib::ustring folder_name = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        if (export2html.prepare_html_folder("", folder_name, false, ret_html_path))
            export2html.node_export_to_html(_pCtMainWin->curr_tree_iter(), _export_options, "", iter_start.get_offset(), iter_end.get_offset());
    }
    if (!ret_html_path.empty())
       CtFileSystem::external_folderpath_open(ret_html_path);
}

// Export To Plain Text Multiple (or single) Files
void CtActions::_export_to_txt(bool is_single, Glib::ustring auto_path, bool auto_overwrite)
{
    if (!_is_there_selected_node_or_error()) return;
    CtDialogs::CtProcessNode export_type;
    if (auto_path != "")
    {
        _export_options.include_node_name = true;
        export_type = CtDialogs::CtProcessNode::ALL_TREE;
    }
    else
        export_type = CtDialogs::selnode_selnodeandsub_alltree_dialog(*_pCtMainWin, true, &_export_options.include_node_name, nullptr, nullptr);
    if (export_type == CtDialogs::CtProcessNode::NONE) return;

    if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE)
    {
        Glib::ustring txt_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        txt_filepath = _get_txt_filepath(txt_filepath);
        if (txt_filepath == "") return;
        CtExport2Txt(_pCtMainWin).node_export_to_txt(_pCtMainWin->curr_tree_iter(), txt_filepath, _export_options, -1, -1);
    }
    else if (export_type == CtDialogs::CtProcessNode::CURRENT_NODE_AND_SUBNODES)
    {
        if (is_single)
        {
           Glib::ustring txt_filepath = _get_txt_filepath(_pCtMainWin->get_ct_storage()->get_file_name());
           if (txt_filepath == "") return;
           CtExport2Txt(_pCtMainWin).nodes_all_export_to_txt(false, "", txt_filepath, _export_options);
        }
        else
        {
            Glib::ustring folder_path = _get_txt_folder("", CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter()), false);
            if (folder_path == "") return;
            CtExport2Txt(_pCtMainWin).nodes_all_export_to_txt(false, folder_path, "", _export_options);
        }
    }
    else if (export_type == CtDialogs::CtProcessNode::ALL_TREE)
    {
        if (is_single)
        {
            Glib::ustring txt_filepath = _get_txt_filepath(_pCtMainWin->get_ct_storage()->get_file_name());
            if (txt_filepath == "") return;
            CtExport2Txt(_pCtMainWin).nodes_all_export_to_txt(true, "", txt_filepath, _export_options);
        }
        else
        {
            Glib::ustring folder_path;
            if (auto_path != "")
                folder_path = _get_txt_folder(auto_path, _pCtMainWin->get_ct_storage()->get_file_name(), auto_overwrite);
            else
                folder_path = _get_txt_folder("", _pCtMainWin->get_ct_storage()->get_file_name(), false);
            if (folder_path == "") return;
            CtExport2Txt(_pCtMainWin).nodes_all_export_to_txt(true, folder_path, "", _export_options);
        }
    }
    else if (export_type == CtDialogs::CtProcessNode::SELECTED_TEXT)
    {
        if (!_is_there_text_selection_or_error()) return;
        Gtk::TextIter iter_start, iter_end;
        _curr_buffer()->get_selection_bounds(iter_start, iter_end);

        Glib::ustring txt_filepath = CtMiscUtil::get_node_hierarchical_name(_pCtMainWin->curr_tree_iter());
        txt_filepath = _get_txt_filepath(txt_filepath);
        if (txt_filepath == "") return;
        CtExport2Txt(_pCtMainWin).node_export_to_txt(_pCtMainWin->curr_tree_iter(), txt_filepath, _export_options, iter_start.get_offset(), iter_end.get_offset());
    }
}

Glib::ustring CtActions::_get_pdf_filepath(Glib::ustring proposed_name)
{
    CtDialogs::file_select_args args(_pCtMainWin);
    args.curr_folder = _pCtMainWin->get_ct_config()->pickDirExport;
    args.curr_file_name = proposed_name + ".pdf";
    args.filter_name = _("PDF File");
    args.filter_pattern = {"*.pdf"};

    Glib::ustring filename = CtDialogs::file_save_as_dialog(args);
    if (filename != "")
    {
        if (!str::endswith(filename, ".pdf")) filename += ".pdf";
        _pCtMainWin->get_ct_config()->pickDirExport = Glib::path_get_dirname(filename);
    }
    return filename;
}

// Prepare for the txt file save
Glib::ustring CtActions::_get_txt_filepath(Glib::ustring proposed_name)
{
    CtDialogs::file_select_args args(_pCtMainWin);
    args.curr_folder = _pCtMainWin->get_ct_config()->pickDirExport;
    args.curr_file_name = proposed_name + ".txt";
    args.filter_name = _("Plain Text Document");
    args.filter_pattern = {"*.txt"};

    Glib::ustring filename = CtDialogs::file_save_as_dialog(args);
    if (filename != "")
    {
        if (!str::endswith(filename, ".txt")) filename += ".txt";
        _pCtMainWin->get_ct_config()->pickDirExport = Glib::path_get_dirname(filename);

        if (Glib::file_test(filename, Glib::FILE_TEST_IS_REGULAR))
            g_remove(filename.c_str());
    }
    return filename;
}

Glib::ustring CtActions::_get_txt_folder(Glib::ustring dir_place, Glib::ustring new_folder, bool export_overwrite)
{
    if (dir_place == "")
    {
        dir_place = CtDialogs::folder_select_dialog(_pCtMainWin->get_ct_config()->pickDirExport, _pCtMainWin);
        if (dir_place == "")
            return "";
    }
    new_folder = CtMiscUtil::clean_from_chars_not_for_filename(new_folder) + "_TXT";
    new_folder = CtFileSystem::prepare_export_folder(dir_place, new_folder, export_overwrite);
    Glib::ustring export_dir = Glib::build_filename(dir_place, new_folder);
    g_mkdir_with_parents(export_dir.c_str(), 0777);

    return export_dir;
}
