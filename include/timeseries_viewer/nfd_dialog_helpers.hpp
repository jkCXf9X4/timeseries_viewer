#pragma once

#include <optional>

#include <nfd.h>

namespace tsv::dialog {

inline nfdopendialogu8args_t make_open_dialog_args(
  const nfdu8filteritem_t* filter_list,
  nfdfiltersize_t filter_count,
  const nfdu8char_t* default_path,
  std::optional<nfdwindowhandle_t> parent_window = std::nullopt) {
  return nfdopendialogu8args_t{
    filter_list,
    filter_count,
    default_path,
    parent_window.value_or(nfdwindowhandle_t{})
  };
}

inline nfdsavedialogu8args_t make_save_dialog_args(
  const nfdu8filteritem_t* filter_list,
  nfdfiltersize_t filter_count,
  const nfdu8char_t* default_path,
  const nfdu8char_t* default_name,
  std::optional<nfdwindowhandle_t> parent_window = std::nullopt) {
  return nfdsavedialogu8args_t{
    filter_list,
    filter_count,
    default_path,
    default_name,
    parent_window.value_or(nfdwindowhandle_t{})
  };
}

} // namespace tsv::dialog
