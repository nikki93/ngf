version 6.0
if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
map! <S-Insert> <MiddleMouse>
map Q gq
nmap gx <Plug>NetrwBrowseX
map <S-Insert> <MiddleMouse>
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
inoremap  u
let &cpo=s:cpo_save
unlet s:cpo_save
set background=dark
set backspace=indent,eol,start
set cmdheight=2
set fileencodings=ucs-bom,utf-8,default,latin1
set guifont=Monospace\ 8
set guioptions=aegimrLt
set helplang=en
set history=50
set hlsearch
set incsearch
set mouse=a
set ruler
set showcmd
set termencoding=utf-8
set window=48
" vim: set ft=vim :
