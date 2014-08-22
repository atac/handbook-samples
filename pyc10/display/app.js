
interval = 50;
data = [];
c10 = [];
channels = {};
plot = null;
current_data = [];
graph_size = 640;
running = true;

function tab(to){
    $('.tab').hide();
    $('#' + to).show();
    $('nav a.active').removeClass('active');
    $('nav a.' + to).addClass('active');
}

$(document).ready(function(){
    plot = $.plot($("#graph"), data, {
        series: {
            shadowSize: 0	// Drawing is faster without shadows
        },
        yaxis: {
            min: 0,
            max: 500
        },
        xaxis: {
            show: false
        }
    });

    $(window).resize(function(e){
        $('#graph').css('height', $('body').height() - 60 + 'px');
    });

    $('body').resize();


    $('#stop').click(function(){
        running = false;
        $(this).attr('disabled', 'disabled');
        $('#start').removeAttr('disabled');
    });

    $('#start').click(function(){
        running = true;
        refresh();
        $(this).attr('disabled', 'disabled');
        $('#stop').removeAttr('disabled');
    });

    var nav = $('nav');
    var template = nav.find('a').remove();
    $('.tab').each(function(i, t){
        var t = $(t);
        nav.append(template.clone().addClass(t.attr('id')).text(t.attr('title')));
        t.removeAttr('title');
    });
    $('nav a').click(function(e){
        e.preventDefault();
        tab($(this).attr('class'));
    });

    cell_template = $('#packet-table tbody tr').remove();

    tab('main');
    refresh();
});

function refresh(){
    $.getJSON('index.json').success(function(result){
        $.each(result.channels, function(i, channel){
            if (channels[channel['id']]){
                channels[channel['id']]['size']++;
            }
            else {
                channels[channel['id']] = channel;
            }
        });

        for (id in channels){
            var row = $('#packet-table tr.channel-' + id);
            if (row.length < 1){
                var row = cell_template.clone();
                row.addClass('channel-' + id);
            }
            row.find('.channel').text(id);
            row.find('.type').text(channels[id]['type']);
            row.find('.size').text(channels[id]['size']);
            $('#packet-table tbody').append(row);
        }

        if ($('#packet-table tbody tr').length > 100){
            $('#packet-table tbody tr').slice(100).remove();
        }

        if (data.length > 0){
            data.concat(result.pcm);
        }
        else {
            data = result.pcm;
        }
    });

    packet = data.shift();
    if (packet != undefined){
        $('#seconds').text(packet.seconds);
        $('#ramp').text(packet.ramp);
        current_data.push(packet);
    }

    var graph_data = current_data.slice(0);
    var graph = [];
    //graph_data.reverse();
    if (graph_data.length < graph_size){
        for (i=0;i<=(graph_size-graph_data.length);i++){
            graph.push([i, 0]);
        }
    }
    $.each(graph_data, function(x, p){
        var i = graph.length + 1;
        graph.push([i, p.ramp]);
    });
    if (graph.length > graph_size){
        graph = graph.slice(-(graph_size - graph.length));
    }
    plot.setData([graph]);
    plot.setupGrid();
    plot.draw();

    if (running){
        setTimeout(refresh, interval);
    }
}
