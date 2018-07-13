<?php
$lang = strtolower($_SERVER['HTTP_ACCEPT_LANGUAGE']);

if ((substr_compare($lang, 'zh-hans', 0, 7, true) == 0) and (substr_compare($lang, 'zh-cn', 0, 5, true) == 0)) {
        include($_SERVER['DOCUMENT_ROOT']."/zh-cn/index.html");
} elseif ((substr_compare($lang, 'zh-hant', 0, 7, true) == 0) and (substr_compare($lang, 'zh-tw', 0, 5, true) == 0)) {
        include($_SERVER['DOCUMENT_ROOT']."/zh-tw/index.html");
} else {
        include($_SERVER['DOCUMENT_ROOT']."/en/index.html");
}
?>