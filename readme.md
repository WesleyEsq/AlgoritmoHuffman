---
## Documentación tecnica:
---
### 0- Como compilarlo
gcc main.c readFile.c tree.c -o main

### 1- Archivos
  
  Existen 4 archivos importantes, estos serían:  
   
+ **tree.c**
	* Contiene funciones para hacer funcionar el arbol del algoritmo de huffman
	* También tiene un min heap para organizar los nodos para comprimirlo
+ **readFile.c**
	* Contiene funciones para leer y comprimir el archivo
	* La función que importa es una llamada "compress file"
+ **tree.h**
	* Header para los dos archivos de arriba.
	* Tiene los structs usados en el algoritmo.
+ **main.c**
	* Aquí iría la ejecución del algoritmo, hacer un programa sencillo de consola


### 2- Funciones del algoritmo de compresión
Se tiene en total alrededor de 500 lineas de código entre ambos archivos para comprimir un archivo y dejar el output en otro en la función de comprimir. Hay un par de algoritmos que pueden ser reutilizables o de importancia para después que debo documentar aquí.

+ **CompressFile():** Es una función para comprimir el archivo como ya fue mencionada. Está para comprimir el archivo en otro de output, así sin más. La compresión incluye lo siguiente:

	* Primero la cantidad total de caracteres que habían en el archivo original, para que descomprimirlo sea más sencillo (Se sepa cuando parar de descomprimir)
	* Segundo la frecuencia del uso de los caracteres en el archivo original. Se debe leer esto después y con ello volver a generar los códigos
	
+ **obtenerCantidadDeCaracteres:** Obtiene el numero que dije arriba
+  **obtenerTablaDeFrecuencias:** Obtiene el uso de los caracteres (la frecuencia).
+  **obtenerCodigosDelComprimido:** Toma lo que produjo la función de "obtener tabla de frecuencias" y con ella genera los códigos. Asi de sencillo, sin más.

+ **TerribleSort():** Es una versión de selection sort, tiene el proposito de obtener las letras que si son usadas en el archivo y ordenarlas en orden acendente


### 3- ¿Como uso los codigos?

El formato que tienen es estar en una variable así:

+ char** reconstructed_codes = reconstruirCodigos(compressed_file);

Saca los codigos de un archivo comprimido con el algoritmo de compresión. Ahora, para obtener el codigo de un caracter se puede hacer esto:
```
//Para acceder al código de un caracter de un diccionario
char* getCodeOfACharacter(char** huffmanCodes, unsigned char symbol){
    return (huffmanCodes[symbol]);
}
```

Es como una matriz, osea el doble* lo hace parecido en memoria, y funciona como una tabla de hash/diccionario.

OK aqui la pregunta, como sacar el codigo de un caracter.
 Es un arreglo de 256. En C, hay 256 caracteres. Los caracteres se guardan internamente en memoria en formato ASCII (se les asigna un numero que se usa para guardar el caracter en un byte de memoria). Para entender mejor ascii, aquí está una tabla que contiene los caracteres y cada uno su respectivo codigo: https://www.ascii-code.com/
 
 + Entonces hay caracteres del 0 al 255 de codigo.
 + También hay 256 elementos en el "reconstructed codes"
 + Uno calza perfectamente en el otro
 
 La variable "reconstructed codes" es un diccionario, cada fila le pertenece a un caracter. Para acceder a la fila de un caracter se usa el propio caracter directamente. Cuando usas un caracter como un numero, c va a tratar al caracter como su codigo ascii, que sería el indice de la fila del elemento.
 
 Es una tabla de dos columnas. El caracter como llave y un string que contiene el codigo. Puedes acceder al string directamente si haces lo de la función de arriba directamente, o usas esa función.
 
 