function autofill_email() {
    var spans = document.getElementsByTagName("span");
    for (i = 0; i < spans.length; i++) {
        if (spans[i].className != "email_autofill")
            continue;
        spans[i].innerHTML = spans[i].innerHTML.replace(/ at /, "@");
    }
}

function parse_query() {
    var parts = location.search.substr(1).split("&");
    var query = {};

    for (var i = 0; i < parts.length; i++) {
        var variable, value;
        var index = parts[i].indexOf("=");
        if (index < 0) {
            variable = decodeURIComponent(parts[i]);
            value = "";
        } else {
            variable = decodeURIComponent(parts[i].substr(0, index));
            value = decodeURIComponent(parts[i].substr(index + 1));
        }
        query[variable] = value;
    }
    return query;
}

function init_questions(query) {
    var questions = document.getElementById('questions');
    if (!questions)
        return;

    var id = query["q"];
    if (id) {
        var qlist = questions.getElementsByTagName('div');
        for (var i = 0; i < qlist.length; i++) {
            if (qlist[i].id != id) {
                questions.removeChild(qlist[i]);
                i--;
            } else if (query["lec"]) {
                // Append the lecture number if we have it.
                var p = document.createElement("p");
                var b = document.createElement("b");
                if ("textContent" in b)
                    b.textContent = "Lecture " + query["lec"];
                else
                    b.innerText = "Lecture " + query["lec"];
                p.appendChild(b);
                qlist[i].insertBefore(p, qlist[i].firstChild);
            }
        }
    }
}

function date_to_txt(d) {
    var month = ["jan", "feb", "mar", "apr", "may", "jun",
                 "jul", "aug", "sep", "oct", "nov", "dec"];
    return month[d.getMonth()] + ' ' + d.getDate();
}

function find_date_txt(t) {
    var els = document.getElementsByTagName('font');
    for (var i = 0; i < els.length; i++) {
        if (els[i].className == "date" && els[i].innerHTML == t)
            return els[i];
    }
    return null;
}

function highlight_date() {
    var d = new Date();
    if (d.getFullYear() != 2015) {
        return;
    }

    for (var fwd = 0; fwd < 3; fwd++) {
        var t = date_to_txt(d);
        var e = find_date_txt(t);
        if (e) {
            e.parentNode.style.border = '2px solid blue';
            break;
        }
        d.setTime(d.getTime() + 24*60*60*1000);
    }
}

function page_onload() {
    var query = parse_query();
    autofill_email();
    init_questions(query);
    highlight_date();
}
