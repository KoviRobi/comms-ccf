function hideParents() {
    let id = window.location.hash.substring(1);
    if (id == "") {
        return;
    }
    let target = document.getElementById(id);
    console.log(id, target);
    // Parent element of the view is the group, we want to start searching
    // upwards from there on, to ignore children of the group
    target = target?.parentElement;
    while (target = target?.parentElement) {
        target.querySelectorAll(":scope>g>a.obscures").forEach(
            el => el.style.display = "none");
    }
}
// window.addEventListener("pageshow", hideParents);
// // Navigating down
// window.addEventListener("hashchange", hideParents);
// // Navigating back
// window.addEventListener("popstate", hideParents);
// hideParents();
