# webkit-opcodes
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/webkit2-opcodes/total.svg)<br>

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

The WebKit opcodes embed the WebKitGTK Web browser and its JavaScript runtime 
into Csound as a set of opcodes. They enable a Csound orchestra to include 
HTML5 code, including JavaScript code; to open one or more browser windows 
during the Csound performance; to open Internet resources during the Csound 
performance; and to control Csound's performance via a JavaScript JSON-RPC 
proxy for the Csound instance, or to install and/or execute JavaScript in the 
context of a Web page from C++ or Csound orchestra code.

These are the opcodes: 
```
i_webkit_handle webkit_create [, i_rpc_port]
i_page_id webkit_open_uri i_webkit_handle, S_window_title, S_uri, i_width, i_height
i_page_id webkit_open_html i_webkit_handle, S_window_title, S_html, S_base_uri, i_width, i_height
i_result webkit_run_javascript i_webkit_handle, i_page_id, S_javascript_code
```
In addition, the following JavaScript interface is in the 
JavaScript context of each Web page opened by these opcodes. As far as possible 
this interface is the same as that in `csound.hpp`. The methods of the `csound` object 
are:
```
CompileCsdText
CompileOrc
EvalCode
Get0dBFS
GetAudioChannel
GetControlChannel
GetDebug
GetKsmps
GetNchnls
GetNchnlsInput
GetScoreOffsetSeconds
GetScoreTime
GetSr
GetStringChannel
InputMessage
IsScorePending
Message
ReadScore
RewindScore
ScoreEvent
SetControlChannel
SetDebug
SetMessageCallback
SetScoreOffsetSeconds
SetScorePending
SetStringChannel
TableGet
TableLength
TableSet

constructor: function(url)
```

# webkit_create

`webkit_create` - Creates an instance of the WebKitGTK Web browser embedded 
into the Csound performance.

## Description

Creates an instance an instance of the WebKitGTK Web browser embedded 
into the Csound performance. This is not _quite_ a full-featured browser, but 
only because it lacks user controls, user settings, history, a URL entry 
bar, and so on. Such features can, however, be created by means of user-
defined HTML and JavaScript code.

In every other way, the embedded browser is indeed full-featured. It can 
display local or remote Web pages, execute JavaScript, open WebSockets, show 
animated WebGL models, and do many other things.

In particular, the embedded browser can call back into the ongoing Csound 
performance using a subset of the Csound API defined in the `Csound.js` 
script. This script can call many methods of the Csound API as implemented 
in `csound.hpp`, but all functions involving the creation, destruction, 
starting and stopping, or runtime configuration had to be omitted. The 
Csound interface in `Csound.js` communicates with the WebKit opcodes and 
thus with Csound using JSON-RPC and Ajax.

Csound itself, or C++ code compiled using the Clang opcodes for Csound, 
can also execute JavaScript code in the JavaScript context of an opened 
Web page using `webkit_run_javascript` opcode.

Thus, the interface between Csound and the Web pages that Csound creates 
is fully bidirectional.

## Syntax
```
i_browser_handle webkit_create [i_rpc_port]
```
## Initialization

*i_rpc_port* - The number of a port on `localhost` that the Csound proxy will
use for JSON-RPC calls. If omitted, the port defaults to 8383.

*i_browser_handle* - Returns a handle to the newly created browser. 
The other WebKit opcodes must take such a handle as their first pfield. One 
browser can open any number of Web pages, each in its own top-level window.

## Performance

Once created, and whether or not it actually displays any Web pages, the browser 
remains in scope until the end of the Csound performance.

# webkit_open_uri

`webkit_open_uri` - Opens a new top-level window and displays in it the content 
defined in the universal resource identifier. This can be a local file or an 
Internet resource.

Please note, pages opened with this opcode do not have access to Csound. 
For that, you must use `webkit_open_html`. `webkit_open_uri` is primarily 
useful for opening external resources, such as documentation.

## Syntax

i_page_id webkit_open_uri i_webkit_handle, S_window_title, S_uri, i_width, i_height

## Initialization

*i_webkit_handle* - The handle of a browser created by `webkit_create`.

*S_window_title* - The title to be displayed by the top-level browser window.

*S_uri* - The Uniform Resource Identifier of an Internet resource to be loaded by 
the browser.

*i_width* - The width of the top-level browser window in pixels.

*i_height* - The height of the top-level browser window in pixels.

*i_page_id* - Returns an identifier for the Web page that was opened. This 
can be passed to `webkit_run_javascript`.

## Performance

The browser window remains open for the remainder of the Csound performance. 
Window events and JavaScript callbacks within the browser are dispatched every 
kperiod.

Right-clicking on the browser opens a context menu with a command to open the 
browser's inspector, or debugger. It can be used to view HTML and JavaScript 
code, inspect elements of the Document Object Model, and to set breakpoints or 
inspect variables in JavaScript code.

# webkit_open_html

`webkit_open_html` - Opens a new top-level window and displays in it the content 
defined by the S_html parameter, typically a multi-line string constant contained 
within the `{{` and `}}` delimiters.

## Syntax

i_page_id webkit_open_html i_webkit_handle, S_window_title, S_html, S_base_uri, i_width, i_height

## Initialization

*i_webkit_handle* - The handle of a browser created by `webkit_create`.

*S_window_title* - Yhe title to be displayed by the top-level browser window.

*S_html* - A string containing valid HTML5 code, typically a multi-line string 
constant contained within the `{{` and `}}` delimiters.

*S_base_uri* - A Uniform Resource Identifier specify the base from which relative 
URI addresses are found. This will normally be the filesystem directory 
that contains the Csound piece. Additional Web pages, JavaScript files, images, 
and so on can be loaded from the base URI.

*i_width* - The width of the top-level browser window in pixels.

*i_height* - The height of the top-level browser window in pixels.

*i_page_id* - Returns an identifier for the Web page that was opened. This 
can be passed to `webkit_run_javascript`.

## Performance

The browser window remains open for the remainder of the Csound performance. 
Window events and JavaScript callbacks within the browser are dispatched every 
kperiod.

In order for user-defined code to call back into Csound, include the 
`csound.js` script that defines the Csound proxy in the body of your Web page. 

Csound can call directly into the Web page using the `webkit_run_javascript` 
opcode.

Right-clicking on the browser opens a context menu with a command to open the 
browser's inspector, or debugger. It can be used to view HTML and JavaScript 
code, inspect elements of the Document Object Model, and to set breakpoints or 
inspect variables in JavaScript code.

## Example

See `webkit_example.js`.

# webkit_run_javascript

`webkit_run_javascript` - Executes JavaScript source code in the JavaScript 
context of the indicated Web page.

## Description

`webkit_run_javascript` - Executes JavaScript source code in the JavaScript 
context of the indicated Web page. This can be used to inject new JavaScript 
modules, including the `Csound.js` Csound proxy, into existing Web pages, or 
for example to send Csound's runtime messages to the Web page for display there, 
or to send a generated score in JSON format for display on the page. Or it can 
be used to call existing functions in the JavaScript context.

## Initialization

*i_webkit_handle* - The handle of a browser created by `webkit_create`.

*S_window_title* - The title to be displayed by the top-level browser window.

*S_uri* - The Uniform Resource Identifier of an Internet resource to be loaded by 
the browser.

*i_width* - The width of the top-level browser window in pixels.

*i_height* - The height of the top-level browser window in pixels.

*i_page_id* - Returns an identifier for the Web page that was opened. This 
can be passed to `webkit_run_javascript`.

## Performance

The browser window remains open for the remainder of the Csound performance. 
Window events and JavaScript callbacks within the browser are dispatched every 
kperiod.

Right-clicking on the browser opens a context menu with a command to open the 
browser's inspector, or debugger. It can be used to view HTML and JavaScript 
code, inspect elements of the Document Object Model, and to set breakpoints or 
inspect variables in JavaScript code.

# webkit_open_html

`webkit_open_html` - Opens a new top-level window and displays in it the content 
defined by the S_html parameter, typically a multi-line string constant contained 
within the `{{` and `}}` delimiters.

## Syntax

i_result webkit_run_javascript i_webkit_handle, i_page_id, S_javascript_code

## Initialization

*i_webkit_handle* - The handle of a browser created by `webkit_create`.

*i_page_id* - An identifier for the Web page in which the code is to run. This 
identifier is returned from `webkit_open_uri` or `webkit_open_html`.

*S_html* - A string containing valid HTML5 code, typically a multi-line string 
constant contained within the `{{` and `}}` delimiters.

*S_javascript_code* - JavaScript source to be executed immediately in the 
JavaScript context of the indicated browser and Web page. Such code has the 
same scope and priviliges as <script> elements in the Web page.


## Performance

What happens during performance is whatever the JavaScript code does. Such code may 
execute immediately, or it may create a class or library to be invoked later on 
during the performance.
   
# Installation

1. Install Csound.
2. Install the WebKitGTK package and its dependencies.
3. Install [libsonrpccpp](sudo apt-get install libjsonrpccpp-dev libjsonrpccpp-tools), 
   preferably as a system package, e.g. 
   `sudo apt-get install libjsonrpccpp-dev libjsonrpccpp-tools`.
4. Generate the stubs and skeletons for the RPC channel that Web pages displayed 
   by the opcodes use to call Csound:
   ```
   jsonrpcstub --verbose csoundrpc.json --js-client=Csound --cpp-server=CsoundSkeleton
   ```   
4. Build the `webkit_opcodes` plugin opcode library by executing `build.sh`. You may need 
to modify this build script for your system.
5. Test by executing `csound webkit_example.csd`. 

# Credits

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com
