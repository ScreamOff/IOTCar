def generate_header(input_html: str, output_header: str):
    with open(input_html, "r", encoding="utf-8") as html_file:
        html_content = html_file.read()
    
    # Konwertujemy zawartość HTML na format odpowiedni dla C++
    formatted_content = '"' + html_content.replace("\n", "\n" + '"\n"') + '";'
    
    with open(output_header, "w", encoding="utf-8") as header_file:
        header_file.write("#ifndef INDEX_H\n")
        header_file.write("#define INDEX_H\n\n")
        header_file.write("const char index_html[] = \n" + formatted_content + "\n\n")
        header_file.write("#endif // INDEX_H\n")

if __name__ == "__main__":
    generate_header("a.html", "index.h")
