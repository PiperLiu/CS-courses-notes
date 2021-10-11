function g(l) {
    var text = '(<a class="$a" href="guidance.html">"$a"</a>)'.replace(/"\$a"/g, l);
    document.write(text);
}
