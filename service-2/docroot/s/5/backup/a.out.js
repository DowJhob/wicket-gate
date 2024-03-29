var Module = typeof Module !== "undefined" ? Module : {};
var moduleOverrides = {};
var key;
for (key in Module) {
    if (Module.hasOwnProperty(key)) {
        moduleOverrides[key] = Module[key]
    }
}
var arguments_ = [];
var thisProgram = "./this.program";
var quit_ = function(status, toThrow) {
    throw toThrow
};
var ENVIRONMENT_IS_WEB = false;
var ENVIRONMENT_IS_WORKER = true;
var ENVIRONMENT_IS_NODE = false;
var ENVIRONMENT_HAS_NODE = false;
var ENVIRONMENT_IS_SHELL = false;
ENVIRONMENT_IS_WEB = typeof window === "object";
ENVIRONMENT_IS_WORKER = typeof importScripts === "function";
ENVIRONMENT_HAS_NODE = typeof process === "object" && typeof process.versions === "object" && typeof process.versions.node === "string";
ENVIRONMENT_IS_NODE = ENVIRONMENT_HAS_NODE && !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_WORKER;
ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;
if (Module["ENVIRONMENT"]) {
    throw new Error("Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -s ENVIRONMENT=web or -s ENVIRONMENT=node)")
}
var scriptDirectory = "";
function locateFile(path) {
    if (Module["locateFile"]) {
        return Module["locateFile"](path, scriptDirectory)
    }
    return scriptDirectory + path
}
var read_,
    readAsync,
    readBinary,
    setWindowTitle;
if (ENVIRONMENT_IS_NODE) {
    scriptDirectory = __dirname + "/";
    var nodeFS;
    var nodePath;
    read_ = function shell_read(filename, binary) {
        var ret;
        if (!nodeFS)
            nodeFS = require("fs");
        if (!nodePath)
            nodePath = require("path");
        filename = nodePath["normalize"](filename);
        ret = nodeFS["readFileSync"](filename);
        return binary ? ret : ret.toString()
    };
    readBinary = function readBinary(filename) {
        var ret = read_(filename, true);
        if (!ret.buffer) {
            ret = new Uint8Array(ret)
        }
        assert(ret.buffer);
        return ret
    };
    if (process["argv"].length > 1) {
        thisProgram = process["argv"][1].replace(/\\/g, "/")
    }
    arguments_ = process["argv"].slice(2);
    if (typeof module !== "undefined") {
        module["exports"] = Module
    }
    process["on"]("uncaughtException", function(ex) {
        if (!(ex instanceof ExitStatus)) {
            throw ex
        }
    });
    process["on"]("unhandledRejection", abort);
    quit_ = function(status) {
        process["exit"](status)
    };
    Module["inspect"] = function() {
        return "[Emscripten Module object]"
    }
} else if (ENVIRONMENT_IS_SHELL) {
    if (typeof read != "undefined") {
        read_ = function shell_read(f) {
            return read(f)
        }
    }
    readBinary = function readBinary(f) {
        var data;
        if (typeof readbuffer === "function") {
            return new Uint8Array(readbuffer(f))
        }
        data = read(f, "binary");
        assert(typeof data === "object");
        return data
    };
    if (typeof scriptArgs != "undefined") {
        arguments_ = scriptArgs
    } else if (typeof arguments != "undefined") {
        arguments_ = arguments
    }
    if (typeof quit === "function") {
        quit_ = function(status) {
            quit(status)
        }
    }
    if (typeof print !== "undefined") {
        if (typeof console === "undefined")
            console = {};
        console.log = print;
        console.warn = console.error = typeof printErr !== "undefined" ? printErr : print
    }
} else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
    if (ENVIRONMENT_IS_WORKER) {
        scriptDirectory = self.location.href
    } else if (document.currentScript) {
        scriptDirectory = document.currentScript.src
    }
    if (scriptDirectory.indexOf("blob:") !== 0) {
        scriptDirectory = scriptDirectory.substr(0, scriptDirectory.lastIndexOf("/") + 1)
    } else {
        scriptDirectory = ""
    }
    read_ = function shell_read(url) {
        var xhr = new XMLHttpRequest;
        xhr.open("GET", url, false);
        xhr.send(null);
        return xhr.responseText
    };
    if (ENVIRONMENT_IS_WORKER) {
        readBinary = function readBinary(url) {
            var xhr = new XMLHttpRequest;
            xhr.open("GET", url, false);
            xhr.responseType = "arraybuffer";
            xhr.send(null);
            return new Uint8Array(xhr.response)
        }
    }
    readAsync = function readAsync(url, onload, onerror) {
        var xhr = new XMLHttpRequest;
        xhr.open("GET", url, true);
        xhr.responseType = "arraybuffer";
        xhr.onload = function xhr_onload() {
            if (xhr.status == 200 || xhr.status == 0 && xhr.response) {
                onload(xhr.response);
                return
            }
            onerror()
        };
        xhr.onerror = onerror;
        xhr.send(null)
    };
    setWindowTitle = function(title) {
        document.title = title
    }
} else {
    throw new Error("environment detection error")
}
var out = Module["print"] || console.log.bind(console);
var err = Module["printErr"] || console.warn.bind(console);
for (key in moduleOverrides) {
    if (moduleOverrides.hasOwnProperty(key)) {
        Module[key] = moduleOverrides[key]
    }
}
moduleOverrides = null;
if (Module["arguments"])
    arguments_ = Module["arguments"];
if (!Object.getOwnPropertyDescriptor(Module, "arguments"))
    Object.defineProperty(Module, "arguments", {
        get: function() {
            abort("Module.arguments has been replaced with plain arguments_")
        }
    });
if (Module["thisProgram"])
    thisProgram = Module["thisProgram"];
if (!Object.getOwnPropertyDescriptor(Module, "thisProgram"))
    Object.defineProperty(Module, "thisProgram", {
        get: function() {
            abort("Module.thisProgram has been replaced with plain thisProgram")
        }
    });
if (Module["quit"])
    quit_ = Module["quit"];
if (!Object.getOwnPropertyDescriptor(Module, "quit"))
    Object.defineProperty(Module, "quit", {
        get: function() {
            abort("Module.quit has been replaced with plain quit_")
        }
    });
assert(typeof Module["memoryInitializerPrefixURL"] === "undefined", "Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead");
assert(typeof Module["pthreadMainPrefixURL"] === "undefined", "Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead");
assert(typeof Module["cdInitializerPrefixURL"] === "undefined", "Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead");
assert(typeof Module["filePackagePrefixURL"] === "undefined", "Module.filePackagePrefixURL option was removed, use Module.locateFile instead");
assert(typeof Module["read"] === "undefined", "Module.read option was removed (modify read_ in JS)");
assert(typeof Module["readAsync"] === "undefined", "Module.readAsync option was removed (modify readAsync in JS)");
assert(typeof Module["readBinary"] === "undefined", "Module.readBinary option was removed (modify readBinary in JS)");
assert(typeof Module["setWindowTitle"] === "undefined", "Module.setWindowTitle option was removed (modify setWindowTitle in JS)");
if (!Object.getOwnPropertyDescriptor(Module, "read"))
    Object.defineProperty(Module, "read", {
        get: function() {
            abort("Module.read has been replaced with plain read_")
        }
    });
if (!Object.getOwnPropertyDescriptor(Module, "readAsync"))
    Object.defineProperty(Module, "readAsync", {
        get: function() {
            abort("Module.readAsync has been replaced with plain readAsync")
        }
    });
if (!Object.getOwnPropertyDescriptor(Module, "readBinary"))
    Object.defineProperty(Module, "readBinary", {
        get: function() {
            abort("Module.readBinary has been replaced with plain readBinary")
        }
    });
stackSave = stackRestore = stackAlloc = function() {
    abort("cannot use the stack before compiled code is ready to run, and has provided stack access")
};
function warnOnce(text) {
    if (!warnOnce.shown)
        warnOnce.shown = {};
    if (!warnOnce.shown[text]) {
        warnOnce.shown[text] = 1;
        err(text)
    }
}
var asm2wasmImports = {
    "f64-rem": function(x, y) {
        return x % y
    },
    "debugger": function() {
        debugger
    }
};
var functionPointers = new Array(0);
var wasmBinary;
if (Module["wasmBinary"])
    wasmBinary = Module["wasmBinary"];
if (!Object.getOwnPropertyDescriptor(Module, "wasmBinary"))
    Object.defineProperty(Module, "wasmBinary", {
        get: function() {
            abort("Module.wasmBinary has been replaced with plain wasmBinary")
        }
    });
var noExitRuntime;
if (Module["noExitRuntime"])
    noExitRuntime = Module["noExitRuntime"];
if (!Object.getOwnPropertyDescriptor(Module, "noExitRuntime"))
    Object.defineProperty(Module, "noExitRuntime", {
        get: function() {
            abort("Module.noExitRuntime has been replaced with plain noExitRuntime")
        }
    });
if (typeof WebAssembly !== "object") {
    abort("No WebAssembly support found. Build with -s WASM=0 to target JavaScript instead.")
}
var wasmMemory;
var wasmTable;
var ABORT = false;
var EXITSTATUS = 0;
function assert(condition, text) {
    if (!condition) {
        abort("Assertion failed: " + text)
    }
}
function getCFunc(ident) {
    var func = Module["_" + ident];
    assert(func, "Cannot call unknown function " + ident + ", make sure it is exported");
    return func
}
function ccall(ident, returnType, argTypes, args, opts) {
    var toC = {
        "string": function(str) {
            var ret = 0;
            if (str !== null && str !== undefined && str !== 0) {
                var len = (str.length << 2) + 1;
                ret = stackAlloc(len);
                stringToUTF8(str, ret, len)
            }
            return ret
        },
        "array": function(arr) {
            var ret = stackAlloc(arr.length);
            writeArrayToMemory(arr, ret);
            return ret
        }
    };
    function convertReturnValue(ret) {
        if (returnType === "string")
            return UTF8ToString(ret);
        if (returnType === "boolean")
            return Boolean(ret);
        return ret
    }
    var func = getCFunc(ident);
    var cArgs = [];
    var stack = 0;
    assert(returnType !== "array", 'Return type should not be "array".');
    if (args) {
        for (var i = 0; i < args.length; i++) {
            var converter = toC[argTypes[i]];
            if (converter) {
                if (stack === 0)
                    stack = stackSave();
                cArgs[i] = converter(args[i])
            } else {
                cArgs[i] = args[i]
            }
        }
    }
    var ret = func.apply(null, cArgs);
    ret = convertReturnValue(ret);
    if (stack !== 0)
        stackRestore(stack);
    return ret
}
function cwrap(ident, returnType, argTypes, opts) {
    return function() {
        return ccall(ident, returnType, argTypes, arguments, opts)
    }
}
function Pointer_stringify(ptr, length) {
    abort("this function has been removed - you should use UTF8ToString(ptr, maxBytesToRead) instead!")
}
var UTF8Decoder = typeof TextDecoder !== "undefined" ? new TextDecoder("utf8") : undefined;
function UTF8ArrayToString(u8Array, idx, maxBytesToRead) {
    var endIdx = idx + maxBytesToRead;
    var endPtr = idx;
    while (u8Array[endPtr] && !(endPtr >= endIdx))
        ++endPtr;
    if (endPtr - idx > 16 && u8Array.subarray && UTF8Decoder) {
        return UTF8Decoder.decode(u8Array.subarray(idx, endPtr))
    } else {
        var str = "";
        while (idx < endPtr) {
            var u0 = u8Array[idx++];
            if (!(u0 & 128)) {
                str += String.fromCharCode(u0);
                continue
            }
            var u1 = u8Array[idx++] & 63;
            if ((u0 & 224) == 192) {
                str += String.fromCharCode((u0 & 31) << 6 | u1);
                continue
            }
            var u2 = u8Array[idx++] & 63;
            if ((u0 & 240) == 224) {
                u0 = (u0 & 15) << 12 | u1 << 6 | u2
            } else {
                if ((u0 & 248) != 240)
                    warnOnce("Invalid UTF-8 leading byte 0x" + u0.toString(16) + " encountered when deserializing a UTF-8 string on the asm.js/wasm heap to a JS string!");
                u0 = (u0 & 7) << 18 | u1 << 12 | u2 << 6 | u8Array[idx++] & 63
            }
            if (u0 < 65536) {
                str += String.fromCharCode(u0)
            } else {
                var ch = u0 - 65536;
                str += String.fromCharCode(55296 | ch >> 10, 56320 | ch & 1023)
            }
        }
    }
    return str
}
function UTF8ToString(ptr, maxBytesToRead) {
    return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : ""
}
function stringToUTF8Array(str, outU8Array, outIdx, maxBytesToWrite) {
    if (!(maxBytesToWrite > 0))
        return 0;
    var startIdx = outIdx;
    var endIdx = outIdx + maxBytesToWrite - 1;
    for (var i = 0; i < str.length; ++i) {
        var u = str.charCodeAt(i);
        if (u >= 55296 && u <= 57343) {
            var u1 = str.charCodeAt(++i);
            u = 65536 + ((u & 1023) << 10) | u1 & 1023
        }
        if (u <= 127) {
            if (outIdx >= endIdx)
                break;
            outU8Array[outIdx++] = u
        } else if (u <= 2047) {
            if (outIdx + 1 >= endIdx)
                break;
            outU8Array[outIdx++] = 192 | u >> 6;
            outU8Array[outIdx++] = 128 | u & 63
        } else if (u <= 65535) {
            if (outIdx + 2 >= endIdx)
                break;
            outU8Array[outIdx++] = 224 | u >> 12;
            outU8Array[outIdx++] = 128 | u >> 6 & 63;
            outU8Array[outIdx++] = 128 | u & 63
        } else {
            if (outIdx + 3 >= endIdx)
                break;
            if (u >= 2097152)
                warnOnce("Invalid Unicode code point 0x" + u.toString(16) + " encountered when serializing a JS string to an UTF-8 string on the asm.js/wasm heap! (Valid unicode code points should be in range 0-0x1FFFFF).");
            outU8Array[outIdx++] = 240 | u >> 18;
            outU8Array[outIdx++] = 128 | u >> 12 & 63;
            outU8Array[outIdx++] = 128 | u >> 6 & 63;
            outU8Array[outIdx++] = 128 | u & 63
        }
    }
    outU8Array[outIdx] = 0;
    return outIdx - startIdx
}
function stringToUTF8(str, outPtr, maxBytesToWrite) {
    assert(typeof maxBytesToWrite == "number", "stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!");
    return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite)
}
var UTF16Decoder = typeof TextDecoder !== "undefined" ? new TextDecoder("utf-16le") : undefined;
function writeArrayToMemory(array, buffer) {
    assert(array.length >= 0, "writeArrayToMemory array must have a length (should be an array or typed array)");
    HEAP8.set(array, buffer)
}
var WASM_PAGE_SIZE = 65536;
var buffer,
    HEAP8,
    HEAPU8,
    HEAP16,
    HEAPU16,
    HEAP32,
    HEAPU32,
    HEAPF32,
    HEAPF64;
function updateGlobalBufferAndViews(buf) {
    buffer = buf;
    Module["HEAP8"] = HEAP8 = new Int8Array(buf);
    Module["HEAP16"] = HEAP16 = new Int16Array(buf);
    Module["HEAP32"] = HEAP32 = new Int32Array(buf);
    Module["HEAPU8"] = HEAPU8 = new Uint8Array(buf);
    Module["HEAPU16"] = HEAPU16 = new Uint16Array(buf);
    Module["HEAPU32"] = HEAPU32 = new Uint32Array(buf);
    Module["HEAPF32"] = HEAPF32 = new Float32Array(buf);
    Module["HEAPF64"] = HEAPF64 = new Float64Array(buf)
}
var STACK_BASE = 136096,
    STACK_MAX = 5378976,
    DYNAMIC_BASE = 5378976,
    DYNAMICTOP_PTR = 136064;
assert(STACK_BASE % 16 === 0, "stack must start aligned");
assert(DYNAMIC_BASE % 16 === 0, "heap must start aligned");
var TOTAL_STACK = 5242880;
if (Module["TOTAL_STACK"])
    assert(TOTAL_STACK === Module["TOTAL_STACK"], "the stack size can no longer be determined at runtime");
var INITIAL_TOTAL_MEMORY = Module["TOTAL_MEMORY"] || 16777216;
if (!Object.getOwnPropertyDescriptor(Module, "TOTAL_MEMORY"))
    Object.defineProperty(Module, "TOTAL_MEMORY", {
        get: function() {
            abort("Module.TOTAL_MEMORY has been replaced with plain INITIAL_TOTAL_MEMORY")
        }
    });
assert(INITIAL_TOTAL_MEMORY >= TOTAL_STACK, "TOTAL_MEMORY should be larger than TOTAL_STACK, was " + INITIAL_TOTAL_MEMORY + "! (TOTAL_STACK=" + TOTAL_STACK + ")");
assert(typeof Int32Array !== "undefined" && typeof Float64Array !== "undefined" && Int32Array.prototype.subarray !== undefined && Int32Array.prototype.set !== undefined, "JS engine does not provide full typed array support");
if (Module["wasmMemory"]) {
    wasmMemory = Module["wasmMemory"]
} else {
    wasmMemory = new WebAssembly.Memory({
        "initial": INITIAL_TOTAL_MEMORY / WASM_PAGE_SIZE,
        "maximum": INITIAL_TOTAL_MEMORY / WASM_PAGE_SIZE
    })
}
if (wasmMemory) {
    buffer = wasmMemory.buffer
}
INITIAL_TOTAL_MEMORY = buffer.byteLength;
assert(INITIAL_TOTAL_MEMORY % WASM_PAGE_SIZE === 0);
updateGlobalBufferAndViews(buffer);
HEAP32[DYNAMICTOP_PTR >> 2] = DYNAMIC_BASE;
function writeStackCookie() {
    assert((STACK_MAX & 3) == 0);
    HEAPU32[(STACK_MAX >> 2) - 1] = 34821223;
    HEAPU32[(STACK_MAX >> 2) - 2] = 2310721022
}
function checkStackCookie() {
    var cookie1 = HEAPU32[(STACK_MAX >> 2) - 1];
    var cookie2 = HEAPU32[(STACK_MAX >> 2) - 2];
    if (cookie1 != 34821223 || cookie2 != 2310721022) {
        abort("Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x02135467, but received 0x" + cookie2.toString(16) + " " + cookie1.toString(16))
    }
    if (HEAP32[0] !== 1668509029)
        abort("Runtime error: The application has corrupted its heap memory area (address zero)!")
}
function abortStackOverflow(allocSize) {
    abort("Stack overflow! Attempted to allocate " + allocSize + " bytes on the stack, but stack has only " + (STACK_MAX - stackSave() + allocSize) + " bytes available!")
}
HEAP32[0] = 1668509029;
HEAP16[1] = 25459;
if (HEAPU8[2] !== 115 || HEAPU8[3] !== 99)
    throw "Runtime error: expected the system to be little-endian!";
function abortFnPtrError(ptr, sig) {
    abort("Invalid function pointer " + ptr + " called with signature '" + sig + "'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this). Build with ASSERTIONS=2 for more info.")
}
function callRuntimeCallbacks(callbacks) {
    while (callbacks.length > 0) {
        var callback = callbacks.shift();
        if (typeof callback == "function") {
            callback();
            continue
        }
        var func = callback.func;
        if (typeof func === "number") {
            if (callback.arg === undefined) {
                Module["dynCall_v"](func)
            } else {
                Module["dynCall_vi"](func, callback.arg)
            }
        } else {
            func(callback.arg === undefined ? null : callback.arg)
        }
    }
}
var __ATPRERUN__ = [];
var __ATINIT__ = [];
var __ATMAIN__ = [];
var __ATPOSTRUN__ = [];
var runtimeInitialized = false;
var runtimeExited = false;
function preRun() {
    if (Module["preRun"]) {
        if (typeof Module["preRun"] == "function")
            Module["preRun"] = [Module["preRun"]];
        while (Module["preRun"].length) {
            addOnPreRun(Module["preRun"].shift())
        }
    }
    callRuntimeCallbacks(__ATPRERUN__)
}
function initRuntime() {
    checkStackCookie();
    assert(!runtimeInitialized);
    runtimeInitialized = true;
    callRuntimeCallbacks(__ATINIT__)
}
function preMain() {
    checkStackCookie();
    callRuntimeCallbacks(__ATMAIN__)
}
function postRun() {
    checkStackCookie();
    if (Module["postRun"]) {
        if (typeof Module["postRun"] == "function")
            Module["postRun"] = [Module["postRun"]];
        while (Module["postRun"].length) {
            addOnPostRun(Module["postRun"].shift())
        }
    }
    callRuntimeCallbacks(__ATPOSTRUN__)
}
function addOnPreRun(cb) {
    __ATPRERUN__.unshift(cb)
}
function addOnPostRun(cb) {
    __ATPOSTRUN__.unshift(cb)
}
assert(Math.imul, "This browser does not support Math.imul(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
assert(Math.fround, "This browser does not support Math.fround(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
assert(Math.clz32, "This browser does not support Math.clz32(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
assert(Math.trunc, "This browser does not support Math.trunc(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null;
var runDependencyTracking = {};
function addRunDependency(id) {
    runDependencies++;
    if (Module["monitorRunDependencies"]) {
        Module["monitorRunDependencies"](runDependencies)
    }
    if (id) {
        assert(!runDependencyTracking[id]);
        runDependencyTracking[id] = 1;
        if (runDependencyWatcher === null && typeof setInterval !== "undefined") {
            runDependencyWatcher = setInterval(function() {
                if (ABORT) {
                    clearInterval(runDependencyWatcher);
                    runDependencyWatcher = null;
                    return
                }
                var shown = false;
                for (var dep in runDependencyTracking) {
                    if (!shown) {
                        shown = true;
                        err("still waiting on run dependencies:")
                    }
                    err("dependency: " + dep)
                }
                if (shown) {
                    err("(end of list)")
                }
            }, 1e4)
        }
    } else {
        err("warning: run dependency added without ID")
    }
}
function removeRunDependency(id) {
    runDependencies--;
    if (Module["monitorRunDependencies"]) {
        Module["monitorRunDependencies"](runDependencies)
    }
    if (id) {
        assert(runDependencyTracking[id]);
        delete runDependencyTracking[id]
    } else {
        err("warning: run dependency removed without ID")
    }
    if (runDependencies == 0) {
        if (runDependencyWatcher !== null) {
            clearInterval(runDependencyWatcher);
            runDependencyWatcher = null
        }
        if (dependenciesFulfilled) {
            var callback = dependenciesFulfilled;
            dependenciesFulfilled = null;
            callback()
        }
    }
}
Module["preloadedImages"] = {};
Module["preloadedAudios"] = {};
var FS = {
    error: function() {
        abort("Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with  -s FORCE_FILESYSTEM=1")
    },
    init: function() {
        FS.error()
    },
    createDataFile: function() {
        FS.error()
    },
    createPreloadedFile: function() {
        FS.error()
    },
    createLazyFile: function() {
        FS.error()
    },
    open: function() {
        FS.error()
    },
    mkdev: function() {
        FS.error()
    },
    registerDevice: function() {
        FS.error()
    },
    analyzePath: function() {
        FS.error()
    },
    loadFilesFromDB: function() {
        FS.error()
    },
    ErrnoError: function ErrnoError() {
        FS.error()
    }
};
Module["FS_createDataFile"] = FS.createDataFile;
Module["FS_createPreloadedFile"] = FS.createPreloadedFile;
var dataURIPrefix = "data:application/octet-stream;base64,";
function isDataURI(filename) {
    return String.prototype.startsWith ? filename.startsWith(dataURIPrefix) : filename.indexOf(dataURIPrefix) === 0
}
var wasmBinaryFile = "a.out.wasm";
if (!isDataURI(wasmBinaryFile)) {
    wasmBinaryFile = locateFile(wasmBinaryFile)
}
function getBinary() {
    try {
        if (wasmBinary) {
            return new Uint8Array(wasmBinary)
        }
        if (readBinary) {
            return readBinary(wasmBinaryFile)
        } else {
            throw "both async and sync fetching of the wasm failed"
        }
    } catch (err) {
        abort(err)
    }
}
function getBinaryPromise() {
    if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) && typeof fetch === "function") {
        return fetch(wasmBinaryFile, {
            credentials: "same-origin"
        }).then(function(response) {
            if (!response["ok"]) {
                throw "failed to load wasm binary file at '" + wasmBinaryFile + "'"
            }
            return response["arrayBuffer"]()
        }).catch(function() {
            return getBinary()
        })
    }
    return new Promise(function(resolve, reject) {
        resolve(getBinary())
    })
}
function createWasm(env) {
    var info = {
        "env": env,
        "wasi_unstable": env,
        "global": {
            "NaN": NaN,
            Infinity: Infinity
        },
        "global.Math": Math,
        "asm2wasm": asm2wasmImports
    };
    function receiveInstance(instance, module) {
        var exports = instance.exports;
        Module["asm"] = exports;
        removeRunDependency("wasm-instantiate")
    }
    addRunDependency("wasm-instantiate");
    var trueModule = Module;
    function receiveInstantiatedSource(output) {
        assert(Module === trueModule, "the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?");
        trueModule = null;
        receiveInstance(output["instance"])
    }
    function instantiateArrayBuffer(receiver) {
        return getBinaryPromise().then(function(binary) {
            return WebAssembly.instantiate(binary, info)
        }).then(receiver, function(reason) {
            err("failed to asynchronously prepare wasm: " + reason);
            abort(reason)
        })
    }
    function instantiateAsync() {
        if (!wasmBinary && typeof WebAssembly.instantiateStreaming === "function" && !isDataURI(wasmBinaryFile) && typeof fetch === "function") {
            fetch(wasmBinaryFile, {
                credentials: "same-origin"
            }).then(function(response) {
                var result = WebAssembly.instantiateStreaming(response, info);
                return result.then(receiveInstantiatedSource, function(reason) {
                    err("wasm streaming compile failed: " + reason);
                    err("falling back to ArrayBuffer instantiation");
                    instantiateArrayBuffer(receiveInstantiatedSource)
                })
            })
        } else {
            return instantiateArrayBuffer(receiveInstantiatedSource)
        }
    }
    if (Module["instantiateWasm"]) {
        try {
            var exports = Module["instantiateWasm"](info, receiveInstance);
            return exports
        } catch (e) {
            err("Module.instantiateWasm callback failed with error: " + e);
            return false
        }
    }
    instantiateAsync();
    return {}
}
Module["asm"] = function(global, env, providedBuffer) {
    env["memory"] = wasmMemory;
    env["table"] = wasmTable = new WebAssembly.Table({
        "initial": 24,
        "maximum": 24,
        "element": "anyfunc"
    });
    env["__memory_base"] = 1024;
    env["__table_base"] = 0;
    var exports = createWasm(env);
    assert(exports, "binaryen setup failed (no wasm support?)");
    return exports
};
var tempDoublePtr = 136080;
assert(tempDoublePtr % 8 == 0);
function demangle(func) {
    warnOnce("warning: build with  -s DEMANGLE_SUPPORT=1  to link in libcxxabi demangling");
    return func
}
function demangleAll(text) {
    var regex = /\b__Z[\w\d_]+/g;
    return text.replace(regex, function(x) {
        var y = demangle(x);
        return x === y ? x : y + " [" + x + "]"
    })
}
function jsStackTrace() {
    var err = new Error;
    if (!err.stack) {
        try {
            throw new Error(0)
        } catch (e) {
            err = e
        }
        if (!err.stack) {
            return "(no stack trace available)"
        }
    }
    return err.stack.toString()
}
function stackTrace() {
    var js = jsStackTrace();
    if (Module["extraStackTrace"])
        js += "\n" + Module["extraStackTrace"]();
    return demangleAll(js)
}
function ___assert_fail(condition, filename, line, func) {
    abort("Assertion failed: " + UTF8ToString(condition) + ", at: " + [filename ? UTF8ToString(filename) : "unknown filename", line, func ? UTF8ToString(func) : "unknown function"])
}
var PATH = {
    splitPath: function(filename) {
        var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
        return splitPathRe.exec(filename).slice(1)
    },
    normalizeArray: function(parts, allowAboveRoot) {
        var up = 0;
        for (var i = parts.length - 1; i >= 0; i--) {
            var last = parts[i];
            if (last === ".") {
                parts.splice(i, 1)
            } else if (last === "..") {
                parts.splice(i, 1);
                up++
            } else if (up) {
                parts.splice(i, 1);
                up--
            }
        }
        if (allowAboveRoot) {
            for (; up; up--) {
                parts.unshift("..")
            }
        }
        return parts
    },
    normalize: function(path) {
        var isAbsolute = path.charAt(0) === "/",
            trailingSlash = path.substr(-1) === "/";
        path = PATH.normalizeArray(path.split("/").filter(function(p) {
            return !!p
        }), !isAbsolute).join("/");
        if (!path && !isAbsolute) {
            path = "."
        }
        if (path && trailingSlash) {
            path += "/"
        }
        return (isAbsolute ? "/" : "") + path
    },
    dirname: function(path) {
        var result = PATH.splitPath(path),
            root = result[0],
            dir = result[1];
        if (!root && !dir) {
            return "."
        }
        if (dir) {
            dir = dir.substr(0, dir.length - 1)
        }
        return root + dir
    },
    basename: function(path) {
        if (path === "/")
            return "/";
        var lastSlash = path.lastIndexOf("/");
        if (lastSlash === -1)
            return path;
        return path.substr(lastSlash + 1)
    },
    extname: function(path) {
        return PATH.splitPath(path)[3]
    },
    join: function() {
        var paths = Array.prototype.slice.call(arguments, 0);
        return PATH.normalize(paths.join("/"))
    },
    join2: function(l, r) {
        return PATH.normalize(l + "/" + r)
    }
};
var SYSCALLS = {
    buffers: [null, [], []],
    printChar: function(stream, curr) {
        var buffer = SYSCALLS.buffers[stream];
        assert(buffer);
        if (curr === 0 || curr === 10) {
            (stream === 1 ? out : err)(UTF8ArrayToString(buffer, 0));
            buffer.length = 0
        } else {
            buffer.push(curr)
        }
    },
    varargs: 0,
    get: function(varargs) {
        SYSCALLS.varargs += 4;
        var ret = HEAP32[SYSCALLS.varargs - 4 >> 2];
        return ret
    },
    getStr: function() {
        var ret = UTF8ToString(SYSCALLS.get());
        return ret
    },
    get64: function() {
        var low = SYSCALLS.get(),
            high = SYSCALLS.get();
        if (low >= 0)
            assert(high === 0);
        else
            assert(high === -1);
        return low
    },
    getZero: function() {
        assert(SYSCALLS.get() === 0)
    }
};
function ___syscall140(which, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        var stream = SYSCALLS.getStreamFromFD(),
            offset_high = SYSCALLS.get(),
            offset_low = SYSCALLS.get(),
            result = SYSCALLS.get(),
            whence = SYSCALLS.get();
        abort("it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM");
        return 0
    } catch (e) {
        if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError))
            abort(e);
        return -e.errno
    }
}
function ___syscall6(which, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        var stream = SYSCALLS.getStreamFromFD();
        abort("it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM");
        return 0
    } catch (e) {
        if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError))
            abort(e);
        return -e.errno
    }
}
function _fd_write(stream, iov, iovcnt, pnum) {
    try {
        var num = 0;
        for (var i = 0; i < iovcnt; i++) {
            var ptr = HEAP32[iov + i * 8 >> 2];
            var len = HEAP32[iov + (i * 8 + 4) >> 2];
            for (var j = 0; j < len; j++) {
                SYSCALLS.printChar(stream, HEAPU8[ptr + j])
            }
            num += len
        }
        HEAP32[pnum >> 2] = num;
        return 0
    } catch (e) {
        if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError))
            abort(e);
        return -e.errno
    }
}
function ___wasi_fd_write() {
    return _fd_write.apply(null, arguments)
}
function _emscripten_get_now() {
    abort()
}
function _emscripten_get_now_is_monotonic() {
    return 0 || ENVIRONMENT_IS_NODE || typeof dateNow !== "undefined" || typeof performance === "object" && performance && typeof performance["now"] === "function"
}
function ___setErrNo(value) {
    if (Module["___errno_location"])
        HEAP32[Module["___errno_location"]() >> 2] = value;
    else
        err("failed to set errno from JS");
    return value
}
function _clock_gettime(clk_id, tp) {
    var now;
    if (clk_id === 0) {
        now = Date.now()
    } else if (clk_id === 1 && _emscripten_get_now_is_monotonic()) {
        now = _emscripten_get_now()
    } else {
        ___setErrNo(22);
        return -1
    }
    HEAP32[tp >> 2] = now / 1e3 | 0;
    HEAP32[tp + 4 >> 2] = now % 1e3 * 1e3 * 1e3 | 0;
    return 0
}
function _emscripten_get_heap_size() {
    return HEAP8.length
}
function _js_output_result(symbol, data, polygon, polygon_size) {
    const UTF8ToString = Module["UTF8ToString"];
    const resultView = new Int32Array(Module.HEAP32.buffer, polygon, polygon_size * 2);
    const coordinates = new Int32Array(resultView);
    const downstreamProcessor = Module["processResult"];
    if (downstreamProcessor == null) {
        throw new Error("No downstream processing function set")
    }
    downstreamProcessor(UTF8ToString(symbol), UTF8ToString(data), coordinates)
}
function _llvm_stackrestore(p) {
    var self = _llvm_stacksave;
    var ret = self.LLVM_SAVEDSTACKS[p];
    self.LLVM_SAVEDSTACKS.splice(p, 1);
    stackRestore(ret)
}
function _llvm_stacksave() {
    var self = _llvm_stacksave;
    if (!self.LLVM_SAVEDSTACKS) {
        self.LLVM_SAVEDSTACKS = []
    }
    self.LLVM_SAVEDSTACKS.push(stackSave());
    return self.LLVM_SAVEDSTACKS.length - 1
}
function _emscripten_memcpy_big(dest, src, num) {
    HEAPU8.set(HEAPU8.subarray(src, src + num), dest)
}
function abortOnCannotGrowMemory(requestedSize) {
    abort("Cannot enlarge memory arrays to size " + requestedSize + " bytes (OOM). Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value " + HEAP8.length + ", (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ")
}
function _emscripten_resize_heap(requestedSize) {
    abortOnCannotGrowMemory(requestedSize)
}
if (ENVIRONMENT_IS_NODE) {
    _emscripten_get_now = function _emscripten_get_now_actual() {
        var t = process["hrtime"]();
        return t[0] * 1e3 + t[1] / 1e6
    }
} else if (typeof dateNow !== "undefined") {
    _emscripten_get_now = dateNow
} else if (typeof performance === "object" && performance && typeof performance["now"] === "function") {
    _emscripten_get_now = function() {
        return performance["now"]()
    }
} else {
    _emscripten_get_now = Date.now
}
function nullFunc_ii(x) {
    abortFnPtrError(x, "ii")
}
function nullFunc_iidiiii(x) {
    abortFnPtrError(x, "iidiiii")
}
function nullFunc_iii(x) {
    abortFnPtrError(x, "iii")
}
function nullFunc_iiii(x) {
    abortFnPtrError(x, "iiii")
}
function nullFunc_jiji(x) {
    abortFnPtrError(x, "jiji")
}
function nullFunc_vi(x) {
    abortFnPtrError(x, "vi")
}
function nullFunc_vii(x) {
    abortFnPtrError(x, "vii")
}
var asmGlobalArg = {};
var asmLibraryArg = {
    "abortStackOverflow": abortStackOverflow,
    "nullFunc_ii": nullFunc_ii,
    "nullFunc_iidiiii": nullFunc_iidiiii,
    "nullFunc_iii": nullFunc_iii,
    "nullFunc_iiii": nullFunc_iiii,
    "nullFunc_jiji": nullFunc_jiji,
    "nullFunc_vi": nullFunc_vi,
    "nullFunc_vii": nullFunc_vii,
    "___assert_fail": ___assert_fail,
    "___setErrNo": ___setErrNo,
    "___syscall140": ___syscall140,
    "___syscall6": ___syscall6,
    "___wasi_fd_write": ___wasi_fd_write,
    "_clock_gettime": _clock_gettime,
    "_emscripten_get_heap_size": _emscripten_get_heap_size,
    "_emscripten_memcpy_big": _emscripten_memcpy_big,
    "_emscripten_resize_heap": _emscripten_resize_heap,
    "_js_output_result": _js_output_result,
    "_llvm_stackrestore": _llvm_stackrestore,
    "_llvm_stacksave": _llvm_stacksave,
    "abortOnCannotGrowMemory": abortOnCannotGrowMemory,
    "DYNAMICTOP_PTR": DYNAMICTOP_PTR
};
var asm = Module["asm"](asmGlobalArg, asmLibraryArg, buffer);
Module["asm"] = asm;
var ___errno_location = Module["___errno_location"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["___errno_location"].apply(null, arguments)
};
var _create_buffer = Module["_create_buffer"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["_create_buffer"].apply(null, arguments)
};
var _destroy_buffer = Module["_destroy_buffer"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["_destroy_buffer"].apply(null, arguments)
};
var _scan_image = Module["_scan_image"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["_scan_image"].apply(null, arguments)
};
var establishStackSpace = Module["establishStackSpace"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["establishStackSpace"].apply(null, arguments)
};
var stackAlloc = Module["stackAlloc"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["stackAlloc"].apply(null, arguments)
};
var stackRestore = Module["stackRestore"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["stackRestore"].apply(null, arguments)
};
var stackSave = Module["stackSave"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["stackSave"].apply(null, arguments)
};
var dynCall_vi = Module["dynCall_vi"] = function() {
    assert(runtimeInitialized, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
    assert(!runtimeExited, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
    return Module["asm"]["dynCall_vi"].apply(null, arguments)
};
Module["asm"] = asm;
if (!Object.getOwnPropertyDescriptor(Module, "intArrayFromString"))
    Module["intArrayFromString"] = function() {
        abort("'intArrayFromString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "intArrayToString"))
    Module["intArrayToString"] = function() {
        abort("'intArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "ccall"))
    Module["ccall"] = function() {
        abort("'ccall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
Module["cwrap"] = cwrap;
if (!Object.getOwnPropertyDescriptor(Module, "setValue"))
    Module["setValue"] = function() {
        abort("'setValue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "getValue"))
    Module["getValue"] = function() {
        abort("'getValue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "allocate"))
    Module["allocate"] = function() {
        abort("'allocate' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "getMemory"))
    Module["getMemory"] = function() {
        abort("'getMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "AsciiToString"))
    Module["AsciiToString"] = function() {
        abort("'AsciiToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "stringToAscii"))
    Module["stringToAscii"] = function() {
        abort("'stringToAscii' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "UTF8ArrayToString"))
    Module["UTF8ArrayToString"] = function() {
        abort("'UTF8ArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
Module["UTF8ToString"] = UTF8ToString;
if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF8Array"))
    Module["stringToUTF8Array"] = function() {
        abort("'stringToUTF8Array' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF8"))
    Module["stringToUTF8"] = function() {
        abort("'stringToUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF8"))
    Module["lengthBytesUTF8"] = function() {
        abort("'lengthBytesUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "UTF16ToString"))
    Module["UTF16ToString"] = function() {
        abort("'UTF16ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF16"))
    Module["stringToUTF16"] = function() {
        abort("'stringToUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF16"))
    Module["lengthBytesUTF16"] = function() {
        abort("'lengthBytesUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "UTF32ToString"))
    Module["UTF32ToString"] = function() {
        abort("'UTF32ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF32"))
    Module["stringToUTF32"] = function() {
        abort("'stringToUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF32"))
    Module["lengthBytesUTF32"] = function() {
        abort("'lengthBytesUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "allocateUTF8"))
    Module["allocateUTF8"] = function() {
        abort("'allocateUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "stackTrace"))
    Module["stackTrace"] = function() {
        abort("'stackTrace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "addOnPreRun"))
    Module["addOnPreRun"] = function() {
        abort("'addOnPreRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "addOnInit"))
    Module["addOnInit"] = function() {
        abort("'addOnInit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "addOnPreMain"))
    Module["addOnPreMain"] = function() {
        abort("'addOnPreMain' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "addOnExit"))
    Module["addOnExit"] = function() {
        abort("'addOnExit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "addOnPostRun"))
    Module["addOnPostRun"] = function() {
        abort("'addOnPostRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "writeStringToMemory"))
    Module["writeStringToMemory"] = function() {
        abort("'writeStringToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "writeArrayToMemory"))
    Module["writeArrayToMemory"] = function() {
        abort("'writeArrayToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "writeAsciiToMemory"))
    Module["writeAsciiToMemory"] = function() {
        abort("'writeAsciiToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "addRunDependency"))
    Module["addRunDependency"] = function() {
        abort("'addRunDependency' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "removeRunDependency"))
    Module["removeRunDependency"] = function() {
        abort("'removeRunDependency' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "ENV"))
    Module["ENV"] = function() {
        abort("'ENV' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS"))
    Module["FS"] = function() {
        abort("'FS' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createFolder"))
    Module["FS_createFolder"] = function() {
        abort("'FS_createFolder' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createPath"))
    Module["FS_createPath"] = function() {
        abort("'FS_createPath' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createDataFile"))
    Module["FS_createDataFile"] = function() {
        abort("'FS_createDataFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createPreloadedFile"))
    Module["FS_createPreloadedFile"] = function() {
        abort("'FS_createPreloadedFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createLazyFile"))
    Module["FS_createLazyFile"] = function() {
        abort("'FS_createLazyFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createLink"))
    Module["FS_createLink"] = function() {
        abort("'FS_createLink' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createDevice"))
    Module["FS_createDevice"] = function() {
        abort("'FS_createDevice' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "FS_unlink"))
    Module["FS_unlink"] = function() {
        abort("'FS_unlink' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
    };
if (!Object.getOwnPropertyDescriptor(Module, "GL"))
    Module["GL"] = function() {
        abort("'GL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "dynamicAlloc"))
    Module["dynamicAlloc"] = function() {
        abort("'dynamicAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "loadDynamicLibrary"))
    Module["loadDynamicLibrary"] = function() {
        abort("'loadDynamicLibrary' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "loadWebAssemblyModule"))
    Module["loadWebAssemblyModule"] = function() {
        abort("'loadWebAssemblyModule' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "getLEB"))
    Module["getLEB"] = function() {
        abort("'getLEB' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "getFunctionTables"))
    Module["getFunctionTables"] = function() {
        abort("'getFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "alignFunctionTables"))
    Module["alignFunctionTables"] = function() {
        abort("'alignFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "registerFunctions"))
    Module["registerFunctions"] = function() {
        abort("'registerFunctions' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "addFunction"))
    Module["addFunction"] = function() {
        abort("'addFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "removeFunction"))
    Module["removeFunction"] = function() {
        abort("'removeFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "getFuncWrapper"))
    Module["getFuncWrapper"] = function() {
        abort("'getFuncWrapper' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "prettyPrint"))
    Module["prettyPrint"] = function() {
        abort("'prettyPrint' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "makeBigInt"))
    Module["makeBigInt"] = function() {
        abort("'makeBigInt' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "dynCall"))
    Module["dynCall"] = function() {
        abort("'dynCall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "getCompilerSetting"))
    Module["getCompilerSetting"] = function() {
        abort("'getCompilerSetting' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "stackSave"))
    Module["stackSave"] = function() {
        abort("'stackSave' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "stackRestore"))
    Module["stackRestore"] = function() {
        abort("'stackRestore' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "stackAlloc"))
    Module["stackAlloc"] = function() {
        abort("'stackAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "establishStackSpace"))
    Module["establishStackSpace"] = function() {
        abort("'establishStackSpace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "print"))
    Module["print"] = function() {
        abort("'print' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "printErr"))
    Module["printErr"] = function() {
        abort("'printErr' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "getTempRet0"))
    Module["getTempRet0"] = function() {
        abort("'getTempRet0' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "setTempRet0"))
    Module["setTempRet0"] = function() {
        abort("'setTempRet0' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "callMain"))
    Module["callMain"] = function() {
        abort("'callMain' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
Module["Pointer_stringify"] = Pointer_stringify;
if (!Object.getOwnPropertyDescriptor(Module, "warnOnce"))
    Module["warnOnce"] = function() {
        abort("'warnOnce' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
    };
if (!Object.getOwnPropertyDescriptor(Module, "ALLOC_NORMAL"))
    Object.defineProperty(Module, "ALLOC_NORMAL", {
        get: function() {
            abort("'ALLOC_NORMAL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
        }
    });
if (!Object.getOwnPropertyDescriptor(Module, "ALLOC_STACK"))
    Object.defineProperty(Module, "ALLOC_STACK", {
        get: function() {
            abort("'ALLOC_STACK' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
        }
    });
if (!Object.getOwnPropertyDescriptor(Module, "ALLOC_DYNAMIC"))
    Object.defineProperty(Module, "ALLOC_DYNAMIC", {
        get: function() {
            abort("'ALLOC_DYNAMIC' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
        }
    });
if (!Object.getOwnPropertyDescriptor(Module, "ALLOC_NONE"))
    Object.defineProperty(Module, "ALLOC_NONE", {
        get: function() {
            abort("'ALLOC_NONE' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)")
        }
    });
if (!Object.getOwnPropertyDescriptor(Module, "calledRun"))
    Object.defineProperty(Module, "calledRun", {
        get: function() {
            abort("'calledRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you")
        }
    });
var calledRun;
function ExitStatus(status) {
    this.name = "ExitStatus";
    this.message = "Program terminated with exit(" + status + ")";
    this.status = status
}
dependenciesFulfilled = function runCaller() {
    if (!calledRun)
        run();
    if (!calledRun)
        dependenciesFulfilled = runCaller
};
function run(args) {
    args = args || arguments_;
    if (runDependencies > 0) {
        return
    }
    writeStackCookie();
    preRun();
    if (runDependencies > 0)
        return;
    function doRun() {
        if (calledRun)
            return;
        calledRun = true;
        if (ABORT)
            return;
        initRuntime();
        preMain();
        if (Module["onRuntimeInitialized"])
            Module["onRuntimeInitialized"]();
        assert(!Module["_main"], 'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]');
        postRun()
    }
    if (Module["setStatus"]) {
        Module["setStatus"]("Running...");
        setTimeout(function() {
            setTimeout(function() {
                Module["setStatus"]("")
            }, 1);
            doRun()
        }, 1)
    } else {
        doRun()
    }
    checkStackCookie()
}
Module["run"] = run;
var abortDecorators = [];
function abort(what) {
    if (Module["onAbort"]) {
        Module["onAbort"](what)
    }
    what += "";
    out(what);
    err(what);
    ABORT = true;
    EXITSTATUS = 1;
    var extra = "";
    var output = "abort(" + what + ") at " + stackTrace() + extra;
    if (abortDecorators) {
        abortDecorators.forEach(function(decorator) {
            output = decorator(output, what)
        })
    }
    throw output
}
Module["abort"] = abort;
if (Module["preInit"]) {
    if (typeof Module["preInit"] == "function")
        Module["preInit"] = [Module["preInit"]];
    while (Module["preInit"].length > 0) {
        Module["preInit"].pop()()
    }
}
noExitRuntime = true;
run();

