(function () {
    function makeTablesSortable() {
        const collator = new Intl.Collator(undefined, {
            numeric: true,
            sensitivity: "base"
        });

        const tables = document.querySelectorAll("table");

        tables.forEach(table => {
            const rows = Array.from(table.querySelectorAll("tr"));
            const headerRow = rows.find(tr => tr.querySelector("th"));
            if (!headerRow) return;

            const headers = Array.from(headerRow.querySelectorAll("th"));
            const section = headerRow.parentNode;

            let sortState = {
                column: -1,
                asc: true
            };

            headers.forEach((th, colIndex) => {
                th.style.cursor = "pointer";

                th.addEventListener("click", () => {
                    let asc;

                    if (sortState.column === colIndex) {
                        asc = !sortState.asc;
                    } else {
                        asc = true;
                    }

                    const sectionRows = Array.from(section.children);
                    const headerPos = sectionRows.indexOf(headerRow);
                    if (headerPos === -1) return;

                    const dataRows = sectionRows
                        .slice(headerPos + 1)
                        .filter(tr => tr.matches("tr") && tr.querySelector("td"));

                    dataRows.sort((a, b) => {
                        const aVal = getCellValue(a, colIndex);
                        const bVal = getCellValue(b, colIndex);

                        const aNull = aVal === null;
                        const bNull = bVal === null;

                        if (aNull && bNull) return 0;
                        if (aNull) return asc ? 1 : -1;
                        if (bNull) return asc ? -1 : 1;

                        const aNum = parseSortableNumber(aVal);
                        const bNum = parseSortableNumber(bVal);

                        if (aNum !== null && bNum !== null) {
                            return asc ? aNum - bNum : bNum - aNum;
                        }

                        return asc
                            ? collator.compare(aVal, bVal)
                            : collator.compare(bVal, aVal);
                    });

                    dataRows.forEach(tr => section.appendChild(tr));

                    sortState.column = colIndex;
                    sortState.asc = asc;
                    updateSortIndicators(headers, colIndex, asc);
                });
            });
        });
    }

    function getCellValue(tr, index) {
        const cells = Array.from(tr.children).filter(el => el.tagName === "TD");
        if (index >= cells.length) return null;

        const cell = cells[index];

        if (cell.classList.contains("null")) {
            return null;
        }

        return cell.textContent.trim();
    }

    function parseSortableNumber(text) {
        const normalized = text
            .replace(/\s+/g, "")
            .replace(",", ".");

        if (!normalized || !/^[-+]?\d*\.?\d+$/.test(normalized)) {
            return null;
        }

        const num = Number(normalized);
        return Number.isNaN(num) ? null : num;
    }

    function updateSortIndicators(headers, activeIndex, asc) {
        headers.forEach((th, i) => {
            th.classList.remove("sorted-asc", "sorted-desc");

            if (i === activeIndex) {
                th.classList.add(asc ? "sorted-asc" : "sorted-desc");
            }
        });
    }

    document.addEventListener("DOMContentLoaded", makeTablesSortable);
})();
