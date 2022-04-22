set nu!         " 显示行号
syntax enable
syntax on
colorscheme desert
set autowrite   " 自动保存

" ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ 基本設定 ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑

set nocompatible              " be iMproved, required
filetype off                  " required

" set the runtime path to include Vundle and initialize
set rtp+=~/.vim/bundle/Vundle.vim
call vundle#begin()
" alternatively, pass a path where Vundle should install plugins
"call vundle#begin('~/some/path/here')

" let Vundle manage Vundle, required
Plugin 'VundleVim/Vundle.vim'

" The following are examples of different formats supported.
" Keep Plugin commands between vundle#begin/end.
" plugin on GitHub repo
"Plugin 'tpope/vim-fugitive'
" plugin from http://vim-scripts.org/vim/scripts.html
" Plugin 'L9'
" Git plugin not hosted on GitHub
"Plugin 'git://git.wincent.com/command-t.git'
" git repos on your local machine (i.e. when working on your own plugin)
"Plugin 'file:///home/gmarik/path/to/plugin'
" The sparkup vim script is in a subdirectory of this repo called vim.
" Pass the path to set the runtimepath properly.
"Plugin 'rstacruz/sparkup', {'rtp': 'vim/'}
" Install L9 and avoid a Naming conflict if you've already installed a
" different version somewhere else.
" Plugin 'ascenator/L9', {'name': 'newL9'}

Plugin 'majutsushi/tagbar'
Plugin 'scrooloose/nerdtree'
Plugin 'w0rp/ale'
Plugin 'Valloric/YouCompleteMe'
Plugin 'ludovicchabant/vim-gutentags'


" All of your Plugins must be added before the following line
call vundle#end()            " required
filetype plugin indent on    " required
" To ignore plugin indent changes, instead use:
"filetype plugin on
"
" Brief help
" :PluginList       - lists configured plugins
" :PluginInstall    - installs plugins; append `!` to update or just :PluginUpdate
" :PluginSearch foo - searches for foo; append `!` to refresh local cache
" :PluginClean      - confirms removal of unused plugins; append `!` to auto-approve removal
"
" see :h vundle for more details or wiki for FAQ
" Put your non-Plugin stuff after this line

" ====================================================================

"  自动加载ctags: ctags -R
if filereadable("tags")
      set tags=tags
endif

" 可以查看符號的引用
if has("cscope")
  set csprg=/usr/bin/cscope
  set csto=1
  set cst
  set nocsverb
  " add any database in current directory
  if filereadable("cscope.out")
      cs add cscope.out
  endif
  set csverb
endif

nmap <silent> <F12> :cs find g <C-R>=expand("<cword>")<CR><CR> 

" 透過 Tagbar 開啟視窗，快捷鍵： F8 "
" 這個套件可以檢視程式碼的大綱
nmap <silent><F8> :TagbarToggle<CR>
let g:tagbar_autofocus = 1

" 顯示檔案瀏覽的樹狀目錄 
nmap <silent><F5> :NERDTreeToggle<CR>

" 靜態語法檢測工具
let g:ale_sign_column_always = 1
let g:ale_sign_error = '✗'
let g:ale_sign_warning = 'w'
let g:ale_statusline_format = ['✗ %d', '⚡ %d', '✔ OK']
let g:ale_echo_msg_format = '[%linter%] %code: %%s'
let g:ale_lint_on_text_changed = 'normal'
let g:ale_lint_on_insert_leave = 1
let g:ale_c_gcc_options = '-Wall -O2 -std=c99'
let g:ale_cpp_gcc_options = '-Wall -O2 -std=c++14'
let g:ale_c_cppcheck_options = ''
let g:ale_cpp_cppcheck_options = ''

" 程式自動補全插件 相關設定
let g:ycm_server_python_interpreter='/usr/bin/python'
let g:ycm_global_ycm_extra_conf='~/.vim/.ycm_extra_conf.py'

" 自動非同步更新tags
"  搜索工程目录的标志，碰到这些文件/目录名就停止向上一级目录递归
let g:gutentags_project_root = ['.root', '.svn', '.git', '.hg', '.project', '.gitignore']
"  添加ctags额外参数，会让tags文件变大
"let g:gutentags_ctags_extra_args = ['--fields=+niazlS', '--extra=+q']
"let g:gutentags_ctags_extra_args += ['--c++-kinds=+px']
"let g:gutentags_ctags_extra_args += ['--c-kinds=+px']
let g:gutentags_ctags_extra_args = ['--fields=+lS']
