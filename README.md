### DCT_STEG
Esteganografia nos coeficientes DCTs de arquivos JPEGs com a biblioteca **jpeglib**

O arquivo **dct_steg.cpp** possui 4 métodos, dentre os quais se enontram:

**void write_steg_message(FILE * infile, FILE * outfile, string msg);**
- Insere uma mensagem *msg* num arquivo *infile* e salva o resultado no *outfile*.

**string read_steg_message(FILE * infile);**
- Lê um arquivo *infile* e retorna a string que conseguir ler nele.

Dentro desses métodos você também encontrará, respectivamente, como usar os métodos *write_steg* e *read_steg*, que fazem o trabalho pesado.

O arquivo **dct_steg.h** é só um header mesmo, e o arquivo **main.cpp** possui um programinha simples em interface de texto que utiliza o **dct_steg.cpp** para gravar e ler mensagens, onde o usuário informa a mensagem e os arquivos de entrada e saída.

Tenho rodado no meu computador utilizando o Visual Studio, onde instalei o jpeglib usando o **NuGet** *(Tools -> NuGet Package Manager -> Package Manager Console, e então insira "Install-Package libjpeg")*.

https://www.nuget.org/packages/libjpeg/

Em caso de dúvidas, me contate pelo email **henriquesdj0@gmail.com** - Estou quase sempre online.


##### Próximas implementações:

* Espalhamento dos bits proporcional ao tamanho da imagem
* Esteganografia de arquivos binários
* Uso exclusivo do canal luma (Y')
* Seleção de coeficientes por sub-amostra