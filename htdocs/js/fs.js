var firstTime     = true;
var base_url      = "";
var dirCache      = {};
var curDir        = '/';
var fileListTable = $('#file_list');
var statusLine    = $('#status');

$(document).ready(function() {
    updateFileList();

    $(window).on('hashchange', function() {
        updateFileList();
    });

    $('#select_all').on('change', function() {
        var checked = $(this).prop('checked');
        fileListTable.find('input').prop('checked', checked);
    });

    $('#delete').on('click', function() {
        var files = fileListTable.find('input:checked').map(function(i, val) {
            var name = $(val).attr('data-name');
            if (name.slice(-1) === '/') {
                name = name.slice(0, -1);
            }
            return name;
        });
        if (files.length == 0) {
            alert('Please select at least one file');
            return;
        }
        if (confirm('Are you sure ?')) {
            deleteFiles(files);
        }
    });

    $('#mkdir').on('click', function() {
        var dir = $('#new_folder').val();
        if (dir.length == 0) {
            alert('Please fill out this field.');
            $('#new_folder').focus();
            return;
        }
        createDir(dir);
        $('#new_folder').val('');
    });

    $('#upload').on('click', function() {
        var files = $('#upload_file')[0].files;
        if (files.length == 0) {
            alert('Please select at least one file');
            $('#upload_file').focus();
            return;
        }
        uploadFiles(files);
        $('#upload_file').val('');
    });

    $('#refresh').on('click', function() {
        updateFileList(true);
    });

    $('#home').on('click', function() {
        location.hash = '/';
    });
});

function uploadFiles(files) {
    statusLine.html('').hide();
    blockUI();

    var uploaded = 0;
    var total    = files.length;
    for (var i = 0; i < files.length; i++) {
        var file = files[i];
        xhr = new XMLHttpRequest();
        xhr.file = file; // not necessary if you create scopes like this

        xhr.addEventListener('progress', function(e) {
            var done = e.position || e.loaded, total = e.totalSize || e.total;
            //console.log('xhr progress: ' + (Math.floor(done/total*1000)/10) + '%');
        }, false);

        if ( xhr.upload ) {
            xhr.upload.onprogress = function(e) {
                var done = e.position || e.loaded, total = e.totalSize || e.total;
                //console.log('xhr.upload progress: ' + done + ' / ' + total + ' = ' + (Math.floor(done/total*1000)/10) + '%');
            };
        }

        xhr.onreadystatechange = function(e) {
            if ( 4 == this.readyState ) {
                //console.log(['xhr upload complete', e]);
                if (this.status != 200) {
                    console.log(this.responseText);
                    statusLine.html(statusLine.html() + '<br>' + this.responseText).show();
                }
                uploaded++;
                if (uploaded >= total) {
                    unblockUI();
                    updateFileList(true);
                }
            }
        };

        //console.log('upload file: ' + file.name);
        xhr.open('put', '/api/fs?file=' + encodeURIComponent(curDir + file.name), true);
        xhr.send(file);
    }
}

function createDir(dir) {
    dir = encodeURIComponent(curDir + dir);

    statusLine.html('').hide();
    blockUI();

    $.ajax({
        url:      base_url + '/api/fs?dir=' + dir,
        type:     'PUT',
        success:  function() {
            updateFileList(true);
        },
        error: function(xhr, textStatus) {
            console.log(textStatus);
            statusLine.html(textStatus).show();
        },
        complete: function() {
            unblockUI();
        }
    });
}

function deleteFiles(files) {
    files = $.map(files, function(val, i) {
        return encodeURIComponent(curDir + val);
    });

    statusLine.html('').hide();
    blockUI();

    $.ajax({
        url:      base_url + '/api/fs?file=' + files.join('&file='),
        type:     'DELETE',
        success:  function() {
            updateFileList(true);
        },
        error: function(xhr, textStatus) {
            console.log(textStatus);
            statusLine.html(textStatus).show();
        },
        complete: function() {
            unblockUI();
        }
    });
}

function updateFileList(force) {
    statusLine.html('').hide();

    curDir = location.hash ? location.hash.substr(1) : '/';
    if (force) {
        delete dirCache[curDir];
    }

    $('#title').html('Index of ' + curDir);
    document.title = 'Index of ' + curDir;

    var files = dirCache[curDir];
    if (files) {
        drawFileList(files);
    }
    else {
        blockUI();

        $.ajax({
            url:      base_url + '/api/fs',
            dataType: 'json',
            data:     {dir: curDir},
            type:     'GET',
            success:  function(data) {
                dirCache[curDir] = data.files;
                drawFileList(data.files);
                if (firstTime) {
                    $('#loading').hide();
                    $('#container').show();
                    firstTime = false;
                }
            },
            error: function(xhr, textStatus) {
                console.log(textStatus);
                statusLine.html(textStatus).show();
            },
            complete: function() {
                unblockUI();
            }
        });
    }
}

function drawFileList(fileList) {
    fileListTable.hide().empty();

    if (curDir != '/') {
        var p      = curDir.lastIndexOf('/', curDir.length - 2);
        var parent = '#' + (p > 0 ? curDir.substr(0, p + 1) : '/');
        fileListTable.append(sprintf('\
            <tr>\
                <td>&nbsp;</td>\
                <td><a href="%s"><img src="img/file_type/folder-home.png" alt=""> Parent directory</a></td>\
                <td>&nbsp;</td>\
                <td>&nbsp;</td>\
            </tr>', escapeHtml(parent)));
    }

    fileList.sort(function(a, b) {
        if ( a.isDir && !b.isDir) return -1;
        if (!a.isDir &&  b.isDir) return  1;
        var nameA = a.name.toLowerCase(), nameB = b.name.toLowerCase();
        if (nameA < nameB) return -1
        if (nameA > nameB) return 1
        return 0;
    });

    for (var i = 0; i < fileList.length; i++) {
        file = fileList[i];

        var icon = getIconByFileType(file);
        var name = file.isDir ? file.name + '/' : file.name;
        var href = (file.isDir ? '#' : '') + curDir + name;

        fileListTable.append(sprintf('\
            <tr>\
                <td><input type="checkbox" data-name="%s"></td>\
                <td><a href="%s"><img src="img/file_type/%s" alt=""> %s</a></td>\
                <td>%s</td>\
                <td>%d</td>\
            </tr>',
            escapeHtml(name),
            escapeHtml(href),
            escapeHtml(icon),
            escapeHtml(name),
            file.modifyDateTime,
            file.fileSize));
    }

    fileListTable.show();
}

function sprintf() {
  //  discuss at: http://phpjs.org/functions/sprintf/
  // original by: Ash Searle (http://hexmen.com/blog/)
  // improved by: Michael White (http://getsprink.com)
  // improved by: Jack
  // improved by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // improved by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // improved by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // improved by: Dj
  // improved by: Allidylls
  //    input by: Paulo Freitas
  //    input by: Brett Zamir (http://brett-zamir.me)
  //   example 1: sprintf("%01.2f", 123.1);
  //   returns 1: 123.10
  //   example 2: sprintf("[%10s]", 'monkey');
  //   returns 2: '[    monkey]'
  //   example 3: sprintf("[%'#10s]", 'monkey');
  //   returns 3: '[####monkey]'
  //   example 4: sprintf("%d", 123456789012345);
  //   returns 4: '123456789012345'
  //   example 5: sprintf('%-03s', 'E');
  //   returns 5: 'E00'

  var regex = /%%|%(\d+\$)?([\-+\'#0 ]*)(\*\d+\$|\*|\d+)?(?:\.(\*\d+\$|\*|\d+))?([scboxXuideEfFgG])/g;
  var a = arguments;
  var i = 0;
  var format = a[i++];

  // pad()
  var pad = function(str, len, chr, leftJustify) {
    if (!chr) {
      chr = ' ';
    }
    var padding = (str.length >= len) ? '' : new Array(1 + len - str.length >>> 0)
      .join(chr);
    return leftJustify ? str + padding : padding + str;
  };

  // justify()
  var justify = function(value, prefix, leftJustify, minWidth, zeroPad, customPadChar) {
    var diff = minWidth - value.length;
    if (diff > 0) {
      if (leftJustify || !zeroPad) {
        value = pad(value, minWidth, customPadChar, leftJustify);
      } else {
        value = value.slice(0, prefix.length) + pad('', diff, '0', true) + value.slice(prefix.length);
      }
    }
    return value;
  };

  // formatBaseX()
  var formatBaseX = function(value, base, prefix, leftJustify, minWidth, precision, zeroPad) {
    // Note: casts negative numbers to positive ones
    var number = value >>> 0;
    prefix = (prefix && number && {
      '2'  : '0b',
      '8'  : '0',
      '16' : '0x'
    }[base]) || '';
    value = prefix + pad(number.toString(base), precision || 0, '0', false);
    return justify(value, prefix, leftJustify, minWidth, zeroPad);
  };

  // formatString()
  var formatString = function(value, leftJustify, minWidth, precision, zeroPad, customPadChar) {
    if (precision !== null && precision !== undefined) {
      value = value.slice(0, precision);
    }
    return justify(value, '', leftJustify, minWidth, zeroPad, customPadChar);
  };

  // doFormat()
  var doFormat = function(substring, valueIndex, flags, minWidth, precision, type) {
    var number, prefix, method, textTransform, value;

    if (substring === '%%') {
      return '%';
    }

    // parse flags
    var leftJustify = false;
    var positivePrefix = '';
    var zeroPad = false;
    var prefixBaseX = false;
    var customPadChar = ' ';
    var flagsl = flags.length;
    var j;
    for (j = 0; flags && j < flagsl; j++) {
      switch (flags.charAt(j)) {
      case ' ':
        positivePrefix = ' ';
        break;
      case '+':
        positivePrefix = '+';
        break;
      case '-':
        leftJustify = true;
        break;
      case "'":
        customPadChar = flags.charAt(j + 1);
        break;
      case '0':
        zeroPad = true;
        customPadChar = '0';
        break;
      case '#':
        prefixBaseX = true;
        break;
      }
    }

    // parameters may be null, undefined, empty-string or real valued
    // we want to ignore null, undefined and empty-string values
    if (!minWidth) {
      minWidth = 0;
    } else if (minWidth === '*') {
      minWidth = +a[i++];
    } else if (minWidth.charAt(0) === '*') {
      minWidth = +a[minWidth.slice(1, -1)];
    } else {
      minWidth = +minWidth;
    }

    // Note: undocumented perl feature:
    if (minWidth < 0) {
      minWidth = -minWidth;
      leftJustify = true;
    }

    if (!isFinite(minWidth)) {
      throw new Error('sprintf: (minimum-)width must be finite');
    }

    if (!precision) {
      precision = 'fFeE'.indexOf(type) > -1 ? 6 : (type === 'd') ? 0 : undefined;
    } else if (precision === '*') {
      precision = +a[i++];
    } else if (precision.charAt(0) === '*') {
      precision = +a[precision.slice(1, -1)];
    } else {
      precision = +precision;
    }

    // grab value using valueIndex if required?
    value = valueIndex ? a[valueIndex.slice(0, -1)] : a[i++];

    switch (type) {
    case 's':
      return formatString(String(value), leftJustify, minWidth, precision, zeroPad, customPadChar);
    case 'c':
      return formatString(String.fromCharCode(+value), leftJustify, minWidth, precision, zeroPad);
    case 'b':
      return formatBaseX(value, 2, prefixBaseX, leftJustify, minWidth, precision, zeroPad);
    case 'o':
      return formatBaseX(value, 8, prefixBaseX, leftJustify, minWidth, precision, zeroPad);
    case 'x':
      return formatBaseX(value, 16, prefixBaseX, leftJustify, minWidth, precision, zeroPad);
    case 'X':
      return formatBaseX(value, 16, prefixBaseX, leftJustify, minWidth, precision, zeroPad)
        .toUpperCase();
    case 'u':
      return formatBaseX(value, 10, prefixBaseX, leftJustify, minWidth, precision, zeroPad);
    case 'i':
    case 'd':
      number = +value || 0;
      // Plain Math.round doesn't just truncate
      number = Math.round(number - number % 1);
      prefix = number < 0 ? '-' : positivePrefix;
      value = prefix + pad(String(Math.abs(number)), precision, '0', false);
      return justify(value, prefix, leftJustify, minWidth, zeroPad);
    case 'e':
    case 'E':
    case 'f': // Should handle locales (as per setlocale)
    case 'F':
    case 'g':
    case 'G':
      number = +value;
      prefix = number < 0 ? '-' : positivePrefix;
      method = ['toExponential', 'toFixed', 'toPrecision']['efg'.indexOf(type.toLowerCase())];
      textTransform = ['toString', 'toUpperCase']['eEfFgG'.indexOf(type) % 2];
      value = prefix + Math.abs(number)[method](precision);
      return justify(value, prefix, leftJustify, minWidth, zeroPad)[textTransform]();
    default:
      return substring;
    }
  };

  return format.replace(regex, doFormat);
}

var exts = [
    'archive.png .7z .bz2 .cab .gz .tar',
    'audio.png .aac .aif .aifc .aiff .ape .au .flac .iff .m4a .mid .mp3 .mpa .ra .wav .wma .f4a .f4b .oga .ogg .xm .it .s3m .mod',
    'bin.png .bin .hex',
    'bmp.png .bmp',
    'c.png .c',
    'calc.png .xlsx .xlsm .xltx .xltm .xlam .xlr .xls .csv',
    'cd.png .iso',
    'cpp.png .cpp',
    'css.png .css .sass .scss',
    'deb.png .deb',
    'doc.png .doc .docx .docm .dot .dotx .dotm .log .msg .odt .pages .rtf .tex .wpd .wps',
    'draw.png .svg .svgz',
    'eps.png .ai .eps',
    'exe.png .exe',
    'gif.png .gif',
    'h.png .h',
    'html.png .html .xhtml .shtml .htm .URL .url',
    'ico.png .ico',
    'java.png .jar',
    'jpg.png .jpg .jpeg .jpe',
    'js.png .js .json',
    'markdown.png .md',
    'package.png .pkg .dmg',
    'pdf.png .pdf',
    'php.png .php .phtml',
    'playlist.png .m3u .m3u8 .pls .pls8',
    'png.png .png',
    'ps.png .ps',
    'psd.png .psd',
    'py.png .py',
    'rar.png .rar',
    'rb.png .rb',
    'rpm.png .rpm',
    'rss.png .rss',
    'script.png .bat .cmd .sh',
    'sql.png .sql',
    'tiff.png .tiff .tif',
    'text.png .txt .nfo',
    'video.png .asf .asx .avi .flv .mkv .mov .mp4 .mpg .rm .srt .swf .vob .wmv .m4v .f4v .f4p .ogv',
    'xml.png .xml',
    'zip.png .zip'
];
var icons = {};
for (var i = 0; i < exts.length; i++) {
    var elts = exts[i].split(' ');
    var icon = elts.shift();
    for (var j = 0; j < elts.length; j++) {
        icons[ elts[j] ] = icon;
    }
}
var defaultIcon = 'default.png';

var entityMap = {
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': '&quot;',
    "'": '&#39;',
    "/": '&#x2F;'
  };

function escapeHtml(string) {
    return String(string).replace(/[&<>"'\/]/g, function (s) {
        return entityMap[s];
    });
}

function getIconByFileType(file) {
    var icon;

    if (file.isDir) {
        icon = 'folder.png';
    }
    else {
        var p = file.name.lastIndexOf('.');
        if (p != -1) icon = icons[ file.name.substr(p).toLowerCase() ];
        if (!icon  ) icon = 'default.png';
    }

    return icon;
}

function blockUI() {
    $.blockUI({
        message: '<h1><img src="img/loading.gif" /></h1>',
        timeout: 30000,
        css:     {
            border:          0,
            backgroundColor: 'transparent'
        }
    });
}

function unblockUI() {
    $.unblockUI();
}
