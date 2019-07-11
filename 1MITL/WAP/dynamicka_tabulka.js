var table_body;
var table_column_count;

/* Sort direction */
const SORT_DESC = 1;
const SORT_ASC = 0;

/**
 * Sort items in a table column
 * @param column_id column id
 * @param sort_as numeric/alphabetical sort
 * @param sort_direction ascending/descending sort
 */
function sortColumn(column_id, sort_as, sort_direction) {
    var new_rows = Array.from(table_body.rows);

    if (sort_as === "number") {
        new_rows = new_rows.sort(
                function(a, b) {
                    if (sort_direction === SORT_ASC)
                        return parseInt(a.cells[column_id].textContent.trim()) > parseInt(b.cells[column_id].textContent.trim()) ? 1 : -1;
                    else 
                        return parseInt(b.cells[column_id].textContent.trim()) > parseInt(a.cells[column_id].textContent.trim()) ? 1 : -1;
                }
        );
    } else if (sort_as === "string") {
        new_rows = new_rows.sort(
                function(a, b) {
                    if (sort_direction === SORT_ASC)
                        return a.cells[column_id].textContent.trim().localeCompare(b.cells[column_id].textContent.trim()) > 0 ? 1 : -1;
                    else 
                        return b.cells[column_id].textContent.trim().localeCompare(a.cells[column_id].textContent.trim()) > 0 ? 1 : -1;
                }
        );
    }

    for (var i = 0; i < new_rows.length; ++i) {
        /* Move row at the correct position according to sort result */
        table_body.appendChild(new_rows[i]);
    }
}

/**
 * Hides rows which contain a text of the filter
 */
function filterChanged(is_sensitive) {
    var table_rows = table_body.rows;
    for (var i = 0; i < table_rows.length; ++i) {
        var rowData = table_rows[i];
        var show_row = true;
        for (var j = 0; j < table_column_count; ++j) {
            var filter_input = document.getElementById('filter_' + j).value;
            if (filter_input === "")
                continue
            
            var cell_text = rowData.getElementsByTagName('td')[j].innerHTML;
            if (!is_sensitive) {
                /* lower strings if case insensitive substring searching*/
                cell_text = cell_text.toLowerCase();
                filter_input = filter_input.toLowerCase();
            }
            
            /* Substring searching */
            if (cell_text.indexOf(filter_input) == -1) {
                /* not found, hide row */
                show_row = false;
            }
        }
        
        rowData.style.display = show_row ? "" : "none";
    }

}

/**
* Creates dynamic table with features like filtering and sorting
 * @param table_id table id
 */
function createDynamicTable(table_id) {
    var table = document.getElementById(table_id);
    if (!table)
        return false;

    var table_top;
    var table_head = table.tHead;
    var table_head_cells;
    table_body = table.tBodies[0];
    /* Bail out if no table body */
    if (!table_body) {
        return false;
    }

    var body_row = table_body.rows[0];
    if (!body_row) {
        /* Bail out, no rows in table body */
        return false;
    }

    if (table_head) {
        /* Table has a header, great */
        var head_row = table_head.rows[0];
        if (!head_row) {
            /* No rows in table head, use table body */
            head_row = body_row;
        }
        table_head_cells = head_row.cells;
        table_column_count = table_head_cells.length;
        table_top = table_head;
    } else {
        /* Handle case when table has no header */
        table_column_count = body_row.cells.length;
        table_top = table_body;
    }

    var dynamic_head = document.createElement('thead');
    dynamic_head.setAttribute('id', 'dynamic-head');
    for (var i = 0; i < table_column_count; ++i) {
        var sort_as;
        if (table_head_cells) {
            sort_as = table_head_cells[i].getAttribute('data-sort-as');
        } 

        /* If no value for attribute or table has no header, use alphabetical sort */
        if (!sort_as) {
            sort_as = 'string';
        }

        var sort_desc = sort_as.startsWith("string") ? "abecedne" : "číselne"; 
        var is_sensitive = sort_as.endsWith("sensitive");
        sort_desc += is_sensitive ? " senzitívne" : " nesenzitívne";

        var dynamic_head_content = document.createElement('td');
        dynamic_head_content.innerHTML =  '<div onclick="sortColumn(' + i + ', \'' + sort_as + '\',' + SORT_DESC + ');" class="sort-symbols" title="Zostupne zoradiť (' + sort_desc + ')">&#x21E9;</div>'
        dynamic_head_content.innerHTML += '<div onclick="sortColumn(' + i + ', \'' + sort_as + '\',' + SORT_ASC + ');" class="sort-symbols" title="Vzostupne zoradiť (' + sort_desc + ')">&#x21E7;</div>'
        dynamic_head_content.innerHTML += '<input id="filter_' + i + '" onkeyup="filterChanged(' + is_sensitive + ');" />';
        dynamic_head.appendChild(dynamic_head_content);
    }

    table.insertBefore(dynamic_head, table_top);
    return true;
}