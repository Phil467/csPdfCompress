<h1 align="center">
🧩 csPdfCompress
</h1>

---

This program implements an algorithm for compressing large PDFs. The PDF is loaded, its pages are extracted, and then each page is processed by an algorithm that changes the color of any pixel near a given color to that color. This algorithm can be applied as many times as there are reference pixels. The pages are then each saved as PNG files, and afterwards all are recombined into a PDF. The final PDF is smaller and more readable.
The program uses the Cairo and Poppler libraries to manipulate PDFs, and libpng to manipulate PNGs.
The file ldr_geoph.pdf is used to show how it works. The results are saved in the created folder "ldr_geoph".