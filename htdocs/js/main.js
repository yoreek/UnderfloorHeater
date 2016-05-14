var maxColor               = 26;
var minColor               = 20;
var dayNames               = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
var modes                  = ['off', 'scheduled', 'manual', 'away'];
var hmodes                 = {'off': 0, 'scheduled': 1, 'manual': 2, 'away': 3};
var heaters                = [];
var base_url               = "";
var dashboard;
var saveSettingsHandler;
var saveSettingsLastSerial = 0;
var saveSettingsDelay      = 3000;
var saveSettingsInProgress = false;
var getStateHandler;
var getStateInterval       = 10000;
var getStateInProgress     = false;
var key                    = 1;
var day                    = 0;
var mousedown              = 0;
var editmode               = 'move';
var changed                = 0;

function saveSettings() {
    if (saveSettingsHandler) {
        clearTimeout(saveSettingsHandler);
    }

    saveSettingsHandler    = setTimeout(_saveSettings, saveSettingsDelay);
    saveSettingsInProgress = true;

    function _saveSettings() {
        saveSettingsHandler = null;

        var data = {};
        for (var i = 0; i < heaters.length; i++) {
            heater = heaters[i];
            data['t' + i] = sprintf("%.1f", heater.targetTemperature);
            data['m' + i] = heater.mode;

            var schedule = heater.schedule;
            for (var d = 0; d < schedule.length; d++) {
                var zones = schedule[d];
                for (var z = 0; z < zones.length; z++) {
                    data['s' + i + d + z] = zones[z].start + '-' + zones[z].end + '-' + sprintf("%.2f", zones[z].setpoint);
                }
            }
        }

        var reqSerial = ++saveSettingsLastSerial;
        $.ajax({
            url:      base_url + '/api/heaters/set',
            dataType: 'json',
            data:     data,
            method:   'POST',
            success:  function(data) {
                if (reqSerial == saveSettingsLastSerial) {
                    saveSettingsInProgress = false;
                }
                showState(data);
            },
            error: function() {
                if (reqSerial == saveSettingsLastSerial) {
                    saveSettings();
                }
            }
        });
    }
}

function getState() {
    if (getStateInProgress || saveSettingsInProgress) {
        return;
    }
    getStateInProgress = true;

    $.ajax({
        url:      base_url + '/api/heaters/get',
        dataType: 'json',
        success:  function(data) {
            showState(data);
        },
        complete: function() {
            getStateInProgress = false;
        }
    });
}

function showState(data) {
    for (var i = 0; i < data.heaters.length; i++) {
        var h  = data.heaters[i];
        var el = $("#heater_" + i);
        el.toggleClass('heater-turned', h.turned);
        el.find(".heater-temperature").html(sprintf("%.1f", h.sensor.temperature));
        if (!saveSettingsInProgress) {
            el.find(".heater-setpoint-val").html(sprintf("%.1f", h.targetTemperature));
            el.find(".heater-mode").removeClass('active').filter('[data-mode="' + modes[h.mode] + '"]').addClass('active');
        }
    }
}

function initControls() {
    $("#global_actions").show().find(".heater-mode").click(function(){
        var mode = $(this).attr('data-mode');
        $(this).addClass("active").siblings("button").removeClass("active");
        dashboard.find('.heater-mode[data-mode="' + mode + '"]').trigger('click');
    });

    $("body").on("mousemove", ".slider", function(e) {
        if (mousedown && editmode == 'move') {
            var heater_el = $(this).parents('.heater');
            sliderUpdate(e, heater_el);
        }
    });

    $("body").on("mousedown", ".slider-button", function(e) {
        mousedown = 1;
        key       = $(this).attr('key');
        day       = $(this).parents('.slider').attr('day');
    });

    $("body").mouseup(function(e){
        mousedown = 0;
        if (changed) {
            saveSettings();
            changed = 0;
        }
    });

    $("body").on("touchstart", ".slider-button", function(e) {
        mousedown = 1;
        key       = $(this).attr('key');
        day       = $(this).parents('.slider').attr('day');
    });

    $("body").on("touchend", "body", function(e) {
        mousedown = 0;
        if (changed) {
            saveSettings();
            changed = 0;
        }
    });;

    $("body").on("touchmove", ".slider", function(e) {
        var event = window.event;
        e.pageX   = event.touches[0].pageX;
        if (mousedown && editmode == 'move') {
            day = $(this).attr('day');
            var heater_el = $(this).parents(".heater");
            sliderUpdate(e, heater_el);
        }
    });

    $("body").on("click", ".slider-button", function() {
        if (editmode == 'merge') {
            day           = $(this).parent().attr("day");
            key           = parseInt($(this).attr("key"));
            var heater_el = $(this).parents(".heater");
            var heater_id = heater_el.attr("data-heater-id");
            var schedule  = heaters[heater_id].schedule;

            schedule[day][key - 1].end = schedule[day][key].end;
            schedule[day].splice(key, 1);
            drawDaySlider(heater_el, schedule[day], day);
            //editmode = 'move';
            saveSettings();
        }
    });

    $("body").on("click", ".slider-segment", function(e) {
        day           = $(this).parent().attr("day");
        key           = parseInt($(this).attr("key"));
        var heater_el = $(this).parents(".heater");
        var heater_id = heater_el.attr("data-heater-id");
        var schedule  = heaters[heater_id].schedule;

        if (editmode == 'split') {
            var x           = e.pageX - $(this).parent()[0].offsetLeft;
            var sliderWidth = $(this).parents(".slider").width();
            var prc         = x / sliderWidth;
            var hour        = prc * 24 * 60;
            hour            = Math.round(hour / 30) * 30;

            if (hour > (schedule[day][key].start + 30) && hour < (schedule[day][key].end - 30)) {
                var end = parseFloat(schedule[day][key].end);
                schedule[day][key].end = hour;

                schedule[day].splice(key + 1, 0, {
                    start:    hour,
                    end:      end,
                    setpoint: schedule[day][key].setpoint
                });

                drawDaySlider(heater_el, schedule[day], day);
                saveSettings();
            }
            //editmode = 'move';
        }
        else if (editmode == 'move') {
            heater_el.find(".slider-segment-temperature").val((schedule[day][key].setpoint * 1).toFixed(1));
            heater_el.find(".slider-segment-start").val(formatTime(schedule[day][key].start));
            heater_el.find(".slider-segment-end").val(formatTime(schedule[day][key].end));

            heater_el.find(".slider-segment-block").show();
            heater_el.find(".slider-segment-block-movepos").hide();
        }
    });

    $("body").on("click", ".slider-segment-ok", function() {
        var heater_el = $(this).parents(".heater");
        var heater_id = heater_el.attr("data-heater-id");
        var schedule  = heaters[heater_id].schedule;

        schedule[day][key].setpoint = heater_el.find(".slider-segment-temperature").val();
        var color = colorMap(schedule[day][key].setpoint);
        heater_el.find(".slider[day=" + day + "]").find(".slider-segment[key=" + key + "]").css("background-color", color);

        var time = decodeTime(heater_el.find(".slider-segment-start").val());
        if (time != -1 && key > 0 && key < schedule[day].length) {
            if (time >= (schedule[day][key - 1].start + 30) && time <= (schedule[day][key].end - 30)) {
                schedule[day][key - 1].end   = time;
                schedule[day][key    ].start = time;
            }
        }
        heater_el.find(".slider-segment-start").val(formatTime(schedule[day][key].start));
        updateSliderUI(heater_el, schedule, day, key);

        time = decodeTime(heater_el.find(".slider-segment-end").val());
        if (time != -1 && key > 0 && key < (schedule[day].length - 1)) {
            if (time >= (schedule[day][key].start + 30) && time <= (schedule[day][key + 1].end - 30)) {
                schedule[day][key    ].end   = time;
                schedule[day][key + 1].start = time;
            }
        }
        heater_el.find(".slider-segment-end").val(formatTime(schedule[day][key].end));
        updateSliderUI(heater_el, schedule, day, key + 1);
        saveSettings();
        heater_el.find(".slider-segment-block").hide();
    });

    $(".slider-segment-movepos-ok").click(function() {
        var heater_el = $(this).parents(".heater");
        var heater_id = heater_el.attr("data-heater-id");
        var schedule  = heaters[heater_id].schedule;
        var time      = decodeTime(heater_el.find(".slider-segment-time").val());

        if (time != -1 && key > 0) {
            if (time >= (schedule[day][key - 1].start + 30) && time <= (schedule[day][key].end - 30)) {
                schedule[day][key - 1].end   = time;
                schedule[day][key    ].start = time;
            }
        }
        heater_el.find(".slider-segment-time").val(formatTime(schedule[day][key].start));
        updateSliderUI(heater_el, schedule, day, key);
        heater_el.find(".slider-segment-block-movepos").hide();
        saveSettings();
    });
}

function drawDashboard() {
    dashboard.empty();

    var heater_template = $("#heater_template").eq(0);
    for (var i = 0; i < heaters.length; i++) {
        var h  = heaters[i];
        var el = heater_template.clone();

        el.attr('id', 'heater_' + i);
        el.attr('data-heater-id', i);
        el.toggleClass('heater-turned', h.turned);
        el.find(".heater-title").html(h.name);
        el.find(".heater-setpoint-val").html(sprintf("%.1f", h.targetTemperature));
        el.find(".heater-temperature").html(sprintf("%.1f", h.sensor.temperature));
        el.find(".heater-mode").removeClass('active').filter('[data-mode="' + modes[h.mode] + '"]').addClass('active');
        dashboard.append(el);
        el.show();

        el.find(".heater-setpoint-button").click(function(){
            var heater_el = $(this).parents(".heater");
            var heater_id = heater_el.attr("data-heater-id");
            var heater    = heaters[heater_id];
            var delta     = parseFloat($(this).attr("data-delta"));

            heater_el.find(".heater-mode").removeClass("active").filter('[data-mode="manual"]').addClass("active");
            heater.mode = hmodes["manual"];
            heater.targetTemperature += delta;
            heater_el.find(".heater-setpoint-val").html(sprintf("%.1f", heater.targetTemperature));

            saveSettings();
        });

        el.find(".heater-mode").click(function(){
            var heater_el = $(this).parents(".heater");
            var heater_id = heater_el.attr("data-heater-id");
            var heater    = heaters[heater_id];
            var mode      = $(this).attr('data-mode');

            $(this).addClass("active").siblings("button").removeClass("active");
            heater.mode = hmodes[mode];

            saveSettings();
        });

        el.find(".heater-schedule-button").click(function(){
            $(this).parents(".heater").find('.heater-schedule-block').toggle();
        });

        drawSchedule(el, h.schedule);
    }
}

function initDashboard() {
    drawDashboard();

    initControls();

    updateTime();

    getStateHandler = setInterval(getState, 10000);
    setInterval(updateTime, 10000);
}

$(document).ready(function() {
    dashboard = $("#dashboard");

    $(document).ajaxStart(function() {
        $('.heater').addClass('sync');
    });

    $(document).ajaxStop(function() {
        $('.heater').removeClass('sync');
    });

    $.ajax({ url: base_url + "/api/heaters/get", dataType: "json", success: function(data) {
        heaters = data.heaters;
        initDashboard();
    }});
});

function decodeTime(timestring) {
    var time = -1;

    if (timestring.indexOf(":") != -1) {
        var parts = timestring.split(":");
        var hour  = parseInt(parts[0]);
        var mins  = parseInt(parts[1]);

        if (mins >= 0 && mins < 60 && hour >= 0 && hour < 25) {
            if (hour == 24 && mins != 0) { } else {
                time = hour * 60 + mins
            }
        }
    }

    return time;
}

function drawSchedule(heater_el, schedule) {
    for (var day in schedule)
        drawDaySlider(heater_el, schedule[day], day);
}

function drawDaySlider(heater_el, zones, day) {
    var out = "";
    var key = 0;
    for (z in zones) {
        var left  = (zones[z].start / (24 * 60)) * 100;
        var width = ((zones[z].end - zones[z].start) / (24 * 60)) * 100;
        var color = colorMap(zones[z].setpoint);

        out += "<div class='slider-segment' style='left:" +left + "%;" +
               " width:" + width + "%; background-color:" + color +
               "' key=" + key + " title='" + zones[z].setpoint + "&deg;C'></div>";

        if (z > 0) {
            out += "<div class='slider-button' style='left:" + left + "%;' key=" + key + "></div>";
        }
        key++;
    }

    out += "<div class='slider-label'>" + dayNames[day] + "</div>";
    heater_el.find(".slider[day=" + day + "]").html(out);
}

function updateTime() {
    var now     = (new Date);
    var timenow = now.getHours() * 60 + now.getMinutes();
    var today   = now.getDay() - 1;
    if (today < 0) today = 6;

    for (var i = 0; i < heaters.length; i++) {
        var heater_el = $("#heater_" + i);

        heater_el.find(".timemarker").css({
            top: ((today - 6) * 32 - 22) + "px",
            width: (timenow * 100 / (24 * 60)) + "%"
        });

        heater_el.find(".current-time").html(formatTime(now.getHours() * 60 + now.getMinutes()));
    }
}

function colorMap(temperature) {
    /*
    // http://www.particleincell.com/blog/2014/colormap/
    // rainbow short
    var a=(1-f)/0.25;   //invert and group
    var X=Math.floor(a);    //this is the integer part
    var Y=Math.floor(255*(a-X)); //fractional part from 0 to 255
    switch(X)
    {
        case 0: r=255;g=Y;b=0;break;
        case 1: r=255-Y;g=255;b=0;break;
        case 2: r=0;g=255;b=Y;break;
        case 3: r=0;g=255-Y;b=255;break;
        case 4: r=0;g=0;b=255;break;
    }*/
    var f = (temperature - minColor) / (maxColor - minColor);
    var a = (1 - f);
    var Y = Math.floor(255 * a);
    var r = 255;
    var g = Y;
    var b = 0;
    return "rgb(" + r + ", " + g + ", " + b + ")";
}

function sliderUpdate(e, heater_el) {
    var heater_id = heater_el.attr("data-heater-id");
    var schedule  = heaters[heater_id].schedule;

    heater_el.find(".slider-segment-block-movepos").show();
    heater_el.find(".slider-segment-block").hide();

    if (key != undefined) {
        var x    = e.pageX - heater_el.find(".slider[day=" + day + "]")[0].offsetLeft;
        var prc  = x / heater_el.find(".slider").eq(0).width();
        var hour = prc * 24 * 60;
        hour     = Math.round(hour / 30) * 30;

        if (hour > schedule[day][key - 1].start && hour < schedule[day][key].end) {
            schedule[day][key-1].end = hour;
            schedule[day][key].start = hour;
            updateSliderUI(heater_el, schedule, day, key);
            changed = 1;
        }
        heater_el.find(".slider-segment-time").val(formatTime(schedule[day][key].start));
    }
}

function updateSliderUI(heater_el, schedule, day, key) {
    if (schedule[day] != undefined && key < schedule[day].length) {
        var slider = heater_el.find(".slider[day=" + day + "]");
        if (key > 0) {
            var width = ((schedule[day][key-1].end - schedule[day][key-1].start) / (24 * 60)) * 100;
            slider.find(".slider-segment[key=" + (key - 1) + "]").css("width", width + "%");
        }

        var left  = (schedule[day][key].start / (24 * 60)) * 100;
        var width = ((schedule[day][key].end - schedule[day][key].start) / (24 * 60)) * 100;
        slider.find(".slider-segment[key=" + key + "]").css({
            width: width + "%",
            left:  left + "%"
        }).attr('title', schedule[day][key].setpoint + '°C');
        slider.find(".slider-button[key=" + key + "]").css("left", left + "%");
    }
}

function formatTime(time) {
    var hour = Math.floor(time / 60);
    var mins = time - hour * 60;
    if (mins < 10) mins = "0" + mins;
    return hour + ":" + mins;
}

/*
<input type='file' id='file' name='file' multiple='' />
*/
function uploadFile() {
    var files = document.getElementById('file').files;
    for (var i = 0; i < files.length; i++) {
        var file = files[i];
        xhr = new XMLHttpRequest();
        xhr.file = file; // not necessary if you create scopes like this

        xhr.addEventListener('progress', function(e) {
            var done = e.position || e.loaded, total = e.totalSize || e.total;
            console.log('xhr progress: ' + (Math.floor(done/total*1000)/10) + '%');
        }, false);

        if ( xhr.upload ) {
            xhr.upload.onprogress = function(e) {
                var done = e.position || e.loaded, total = e.totalSize || e.total;
                console.log('xhr.upload progress: ' + done + ' / ' + total + ' = ' + (Math.floor(done/total*1000)/10) + '%');
            };
        }

        xhr.onreadystatechange = function(e) {
            if ( 4 == this.readyState ) {
                console.log(['xhr upload complete', e]);
            }
        };

        xhr.open('put', '/upload/' + file.name, true);
        xhr.send(file);
    }
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
