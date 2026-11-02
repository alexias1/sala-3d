# 🎃 Sala 3D — Halloween | Quebre a Maldição

**Escape room 3D** desenvolvido em **C++17 com OpenGL 3.3**, câmera em primeira pessoa, interações com objetos, enigmas e uma missão de fuga em um cenário assombrado. A sala e o exterior são construídos por código com primitivas, transformações geométricas, texturas, iluminação e shaders GLSL.

**Repositório:** [alexias1/sala-3d](https://github.com/alexias1/sala-3d) · **Branch de referência:** `main`

## Índice

1. [Recursos do jogo](#recursos-do-jogo)
2. [Dependências e requisitos](#dependências-e-requisitos)
3. [Instalação no Windows](#instalação-no-windows)
4. [Instalação no Linux](#instalação-no-linux)
5. [Como executar](#como-executar)
6. [Controles](#controles)
7. [Como jogar e vencer](#como-jogar-e-vencer)
8. [Objetos interativos](#objetos-interativos)
9. [Cenário, texturas e efeitos](#cenário-texturas-e-efeitos)
10. [Organização dos arquivos](#organização-dos-arquivos)
11. [Aspectos técnicos](#aspectos-técnicos)
12. [Problemas comuns](#problemas-comuns)
13. [Limitações e observações](#limitações-e-observações)

## Recursos do jogo

- Exploração de uma sala de Halloween em primeira pessoa, com movimentação por teclado e visão pelo mouse.
- Interações contextuais com a tecla **`E`**, identificadas por prompts no HUD quando o objeto está próximo e sob a mira.
- Missão com **sete objetivos**: acender três velas, coletar três doces, realizar o ritual, pegar uma chave, examinar três lápides, libertar três fantasmas e abrir a porta.
- Três fantasmas com cores, ameaças e mensagens de libertação individuais; os olhos mudam após a libertação.
- Três lápides internas com nomes e epitáfios exclusivos e outras lápides no cemitério externo.
- Cinco abóboras com texturas e rostos luminosos, todas acionando o mesmo interruptor de iluminação.
- Duas bolas de cristal: uma **âmbar**, interativa, na mesa; outra **roxa**, decorativa, na estante. Ambas com textura e alpha blending.
- Tarô com três revelações (passado, presente e futuro) e possibilidade de embaralhar as cartas.
- Tabuleiro Ouija com peça móvel e mensagem misteriosa.
- TV, ventilador, caldeirão, aranha, morcegos, fantasmas e outros elementos animados.
- Exterior com grama texturizada, lua, cemitério, montanhas em 360° e neblina rasteira em camadas.
- HUD com lista de objetivos, contadores, cronômetro, mira, mensagens e tela de vitória.

## Dependências e requisitos

| Item | Requisito / função | Como é fornecido |
| --- | --- | --- |
| **C++17** | Linguagem do jogo | Compilador C++ compatível |
| **OpenGL 3.3 Core Profile** | Renderização 3D | GPU e drivers do sistema |
| **GLSL 330** | Linguagem dos shaders | Suporte da GPU/driver |
| **CMake 3.16+** | Configuração e compilação | Instalar no sistema |
| **Git** | Clonagem e download das dependências | Instalar no sistema |
| **GLFW 3.4** | Janela, contexto OpenGL, teclado e mouse | `FetchContent` do CMake |
| **GLM 1.0.1** | Vetores, matrizes, câmera e transformações | `FetchContent` do CMake |
| **GLAD** | Carregamento de funções OpenGL | Incluído em `external/glad/` |
| **stb_image** | Carregamento de texturas e mapas de normais | Incluído em `external/stb/` |
| **stb_easy_font** | Renderização do texto do HUD | Incluído em `external/stb/` |

**Não é preciso instalar GLFW, GLM, GLAD ou STB separadamente.** O `CMakeLists.txt` baixa as versões especificadas de GLFW e GLM e compila o GLAD que já vem no repositório. A **primeira configuração** requer acesso à internet e ao GitHub.

Outros requisitos: mouse, teclado e uma placa de vídeo com suporte a OpenGL 3.3. O projeto **não usa Unity, Unreal Engine ou outro motor de jogo**.

## Instalação no Windows

Exemplo com **Windows e Visual Studio 2022**. Use o PowerShell na pasta em que deseja salvar o projeto.

### 1. Instalar ferramentas

- [Git](https://git-scm.com/downloads)
- [CMake](https://cmake.org/download/) — instale a versão 3.16 ou superior e habilite seu uso no `PATH`.
- [Visual Studio 2022](https://visualstudio.microsoft.com/) — selecione a carga de trabalho **Desenvolvimento para desktop com C++** (compilador MSVC e ferramentas de build).
- Driver gráfico atualizado com OpenGL 3.3.

**VS Code é opcional** como editor. Se for utilizá-lo, as extensões *C/C++* e *CMake Tools* podem facilitar o desenvolvimento, mas não substituem o compilador.

### 2. Clonar

```powershell
git clone https://github.com/alexias1/sala-3d.git
cd sala-3d
git switch main
```

Se o repositório já está clonado, atualize a branch principal após salvar as mudanças locais:

```powershell
git switch main
git pull origin main
```

### 3. Configurar e compilar

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

O CMake baixa GLFW e GLM na configuração inicial. O executável costuma ser criado em **`build/Release/sala.exe`**.

### 4. Abrir o jogo

```powershell
cd build/Release
./sala.exe
```

Para compilar uma versão de depuração, use `cmake --build build --config Debug`, entre em `build/Debug` e execute `./sala.exe`.

## Instalação no Linux

Exemplo para **Ubuntu/Debian** com ambiente gráfico:

```bash
sudo apt update
sudo apt install -y build-essential cmake git libgl1-mesa-dev xorg-dev

git clone https://github.com/alexias1/sala-3d.git
cd sala-3d
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build
./sala
```

Em algumas distribuições, a compilação da GLFW pode exigir pacotes adicionais de desenvolvimento do **X11/Wayland**. Consulte a mensagem específica do CMake. É preciso executar o jogo em uma sessão gráfica com suporte a OpenGL 3.3. **Esses comandos são orientativos; o projeto não foi testado aqui em Linux.**

## Como executar

1. Compile conforme o sistema operacional.
2. Abra o executável (`sala.exe` no Windows; `sala` no Linux) **a partir da pasta onde ele está localizado**.
3. A janela inicial mede **1280 × 720**. O mouse é capturado para controlar a câmera.
4. Explore a sala, observe o painel de missão no canto superior esquerdo e pressione `E` quando aparecer uma ação contextual.

**Por que executar na pasta do binário?** Os shaders e recursos são abertos com **caminhos relativos**, como `shaders/phong.frag` e `assets/textures/wood.jpg`. O CMake copia as pastas `shaders/` e `assets/` para o diretório do executável através de uma etapa `POST_BUILD`. Se editar um shader ou textura, compile novamente (ou copie a versão atualizada para a pasta de execução).

## Controles

| Controle | Ação |
| --- | --- |
| `W` ou `↑` | Avançar |
| `S` ou `↓` | Recuar |
| `A` ou `←` | Mover para a esquerda |
| `D` ou `→` | Mover para a direita |
| **Mouse** | Girar a câmera / olhar ao redor |
| **`E`** | Interagir com o objeto na mira |
| **`Esc`** | Encerrar o jogo |

**Para interagir:** aproxime-se, mire com o `+` central e aguarde o prompt `[E] ...`. O alcance padrão é de **1,9 unidade do mundo**, configurado por `REACH` em `src/main.cpp`. A câmera se move horizontalmente e possui colisões com móveis e objetos do cenário.

## Como jogar e vencer

**Objetivo principal: quebrar a maldição e abrir a porta da sala.** O HUD mantém uma lista de sete objetivos e marca aqueles que já foram concluídos.

### Passo a passo recomendado

1. **Acender as três velas.** Procure as três velas na mesa central. Aproxime-se de cada uma, mire e pressione `E`. A cada chama acesa, o contador é atualizado.
2. **Encontrar três doces.** Um doce fica próximo ao móvel da TV, outro na estante em formato de caixão e o terceiro no chão, na região da grande abóbora e das lápides. Pressione `E` para coletá-los.
3. **Completar o ritual no caldeirão.** Com as três velas acesas e os três doces coletados, interaja com o caldeirão para realizar a oferenda. Uma chave aparece.
4. **Pegar a chave.** Aproxime-se do caldeirão e pressione `E` sobre a chave.
5. **Examinar as três lápides.** Leia as inscrições das três lápides dentro da sala. O jogo registra apenas a primeira leitura de cada uma; releituras são permitidas.
6. **Libertar os três fantasmas.** Após examinar todas as lápides, procure cada espírito e interaja. Eles deixam de ter olhos vermelhos, recuperam o brilho colorido e apresentam uma despedida diferente.
7. **Abrir a porta.** Com a chave, as três lápides examinadas e os três fantasmas libertados, mire na porta e pressione `E` para vencer.

### Regras importantes

- Para concluir o ritual, são necessárias **três velas acesas e três doces coletados**.
- Antes de examinar todas as lápides, os fantasmas **ainda não podem ser libertados**: respondem apenas com frases ameaçadoras.
- A porta só abre quando os três requisitos estão satisfeitos: **chave + três lápides examinadas + três espíritos libertados**.
- **A vitória ocorre assim que a porta é aberta** com todos os requisitos cumpridos. Não é necessário atravessá-la fisicamente.
- O cronômetro para no instante da fuga e o jogo mostra **“VOCE ESCAPOU!”**, seguido do tempo da partida.
- É possível visitar e interagir com objetos fora da ordem sugerida. O HUD mostra os objetivos que ainda faltam.

## Objetos interativos

| Objeto | O que acontece ao interagir |
| --- | --- |
| **Porta** | Informa o requisito pendente ou abre com animação depois de completar a missão. |
| **TV** | Liga/desliga a tela azulada e sua iluminação, com oscilação de brilho. |
| **Ventilador de teto** | Liga/desliga; a rotação acelera e desacelera gradualmente. |
| **Bola de cristal âmbar** | Liga/desliga a luz quente da bola na mesa. Mantém sua textura e transparência. |
| **Bola de cristal roxa** | **Decorativa**, fica na estante-caixão; não possui interação própria. |
| **Cinco abóboras** | Todas são interativas; qualquer uma acende/apaga **todas as cinco**, incluindo seus rostos luminosos. |
| **Três velas** | Acendem individualmente e contribuem para o ritual. |
| **Três doces** | São coletados e desaparecem do cenário; o progresso aparece no HUD. |
| **Caldeirão** | Mostra pistas sobre o ritual e revela uma chave quando os requisitos são cumpridos. |
| **Chave** | Pode ser coletada após a conclusão do ritual. |
| **Três lápides internas** | Exibem o nome e o epitáfio de cada personagem; podem ser relidas. |
| **Três fantasmas** | Ameaçam o jogador enquanto presos e apresentam falas próprias quando libertados. |
| **Tarô** | Revela passado, presente e futuro e permite embaralhar as cartas novamente. |
| **Ouija** | Move a peça sobre o tabuleiro e exibe uma mensagem misteriosa. |
| **Janela** | Mostra uma mensagem sobre os olhos que observam entre as lápides do lado de fora. |
| **Pentagrama** | Oferece uma pista sobre o ritual das três chamas. |

### Personagens e inscrições

Cada lápide corresponde a um dos espíritos do jogo:

| Espírito | Epitáfio |
| --- | --- |
| **Alexia Rodrigues** | “Nem a morte conseguiu silenciar seus gritos.” |
| **Bárbara Albuquerque** | “Seu corpo encontrou descanso. Sua alma, jamais.” |
| **Lara Nogueira** | “A eternidade tornou-se sua prisão.” |

Os diálogos de aprisionamento e libertação são diferentes para cada personagem. As lápides e os fantasmas compartilham a mesma ordem de identificação definida no código. O tarô tem um conjunto de **seis cartas**, do qual são escolhidas três para cada sequência de revelações; embaralhar permite uma nova leitura.

> O **tarô**, o **Ouija**, a **TV**, o **ventilador**, a **janela**, o **pentagrama**, as **abóboras** e a **bola de cristal** enriquecem a exploração, mas não são obrigatórios para completar os sete objetivos da missão.

## Cenário, texturas e efeitos

### Interior da casa

- Paredes, teto, piso de madeira e janela com moldura.
- Mesa central com as três velas, cartas e bola de cristal âmbar.
- Caldeirão com mistura mágica, partículas luminosas e chave.
- TV, móvel de madeira e ventilador de teto.
- Estante em formato de caixão com prateleiras, livros, caveiras, frascos de poção, cristais e bola roxa.
- Abóboras, lápides, fantasmas, tabuleiro Ouija, doces e pentagrama.
- Teias de aranha, aranha suspensa e morcegos animados.
- Porta de madeira com rotação suave ao abrir.

### Exterior da casa

- **Grama texturizada** envolvendo os quatro lados da casa, inclusive a lateral da porta.
- **Montanhas procedurais** formando um anel de 360°, com encostas, normais calculadas e textura de rocha.
- Lua com textura própria e luz direcional.
- Cemitério externo com três lápides adicionais, textura de pedra e shader de iluminação específico.
- Pares de olhos misteriosos no escuro, com brilho e animação.
- **Neblina rasteira 360°**, com ruído procedural e quatro camadas de alturas/opacidades diferentes, que desaparece antes das montanhas e não invade a sala.
- Feixes de luar translúcidos junto à janela.

### Técnicas gráficas

- **Materiais de iluminação** com componentes ambiente (`ka`), difusa (`kd`), especular (`ks`), brilho especular (`shininess`) e emissão (`emissive`).
- **Phong/Blinn-Phong**, iluminação hemisférica e **luar direcional** com aproximação da entrada de luz pela janela.
- **13 posições de luz de ponto configuradas**, associadas a objetos como TV, velas, abóboras, caldeirão e cristal (algumas permanecem apagadas conforme o estado da cena).
- **Texturas 2D** carregadas por `stb_image`, com repetição UV, mipmaps, filtragem linear e anisotropia quando suportada.
- **Normal mapping** no piso e nas paredes.
- **Correção de gama** e texturas de cor carregadas em formato sRGB.
- **Alpha blending** nas duas bolas de cristal, com tonalidades diferentes e transparência controlada no shader.
- **Ordenação das esferas transparentes** e controle de escrita no depth buffer para deixar os suportes visíveis atrás do cristal.
- **Fog de distância** no shader principal e **neblina rasteira procedural** em um shader próprio.
- Animações baseadas em tempo (`glfwGetTime()`), incluindo ventoinha, movimento dos fantasmas, chamas, morcegos e ponteiro do Ouija.

**Nota:** o alpha blending simula translucidez; o projeto **não implementa refração física ou ray tracing**. As bolas recebem a mesma textura, com tonalidade roxa ou âmbar definida no shader. A bola âmbar evita receber sua própria luz de ponto na superfície para não perder detalhes da textura quando acesa.

## Organização dos arquivos

Estrutura resumida, baseada no `CMakeLists.txt` e nos arquivos utilizados pelo código:

```text
sala-3d/
├── CMakeLists.txt
├── src/
│   ├── main.cpp                # Lógica, cena, animações, interações e HUD
│   ├── Camera.h                # Câmera FPS e vetores de orientação
│   ├── Shader.h                # Compilação, link e uniforms de shaders
│   ├── Mesh.h                  # VAO/VBO/EBO, primitivas e malhas
│   ├── Material.h              # Materiais e parâmetros de iluminação
│   ├── Texture.h               # Texturas, mipmaps e mapas de normais
│   └── Hud.h                   # Interface 2D com stb_easy_font
├── shaders/
│   ├── phong.vert              # Vertex shader principal
│   ├── phong.frag              # Luzes, materiais, texturas e cristais
│   ├── graveyard.frag          # Iluminação das lápides externas
│   ├── ground_fog.frag         # Neblina baixa procedural
│   ├── eyes_glow.frag          # Olhos no cemitério
│   ├── moon_beam.frag          # Feixes de luar
│   ├── hud.vert                # Vertex shader do HUD
│   └── hud.frag                # Fragment shader do HUD
├── assets/
│   └── textures/              # Imagens usadas no cenário e nos objetos
├── external/
│   ├── glad/                  # Headers e external/glad/src/glad.c
│   └── stb/                   # stb_image.h e stb_easy_font.h
└── build/                     # Gerado pelo CMake; ignorado no Git
```

### Arquivos de textura

| Arquivo em `assets/textures/` | Uso |
| --- | --- |
| `wood.jpg` | Madeira de móveis, prateleiras e porta |
| `floor.jpg` | Piso da sala |
| `wall.jpg` | Paredes |
| `floor_normal.jpg` | Mapa de normais do piso |
| `wall_normal.jpg` | Mapa de normais das paredes |
| `textura_lua.jpg` | Lua do cenário externo |
| `textura_montanha.jpeg` | Rochas das montanhas |
| `textura_abobora.png` | **Apenas a casca laranja** das abóboras (não o cabinho ou rosto) |
| `textura_lapides.png` | Corpo e topo das lápides internas e externas |
| `textura_cristal.png` | Superfície das bolas roxa e âmbar |
| `textura_grama.jpg`, `.jpeg` ou `.png` | Gramado externo: o código procura a primeira extensão disponível |
| `ouija_board.png` | Superfície do tabuleiro Ouija |
| `ouija_planchette.png` | Ponteiro móvel do Ouija |

**Atenção:** o código da `main` utiliza o nome exato `textura_montanha.jpeg`. Se você renomear uma imagem, atualize também seu caminho em `src/main.cpp`. A textura da grama é a exceção: o programa testa `.jpg`, `.jpeg` e `.png` automaticamente.

## Aspectos técnicos

### Malhas e transformações

`src/Mesh.h` disponibiliza **cubo, plano, cilindro e esfera**, com vértices, normais e UVs. Objetos mais complexos são montados combinando essas primitivas. O projeto também gera geometrias específicas por código, como as encostas das montanhas, os feixes de luar e o ponteiro vazado do Ouija.

As transformações usam matrizes GLM de **translação, rotação e escala**. A função `makeModel()` concentra essa operação para boa parte dos objetos.

### Câmera, colisões e interações

A câmera (`Camera.h`) usa **yaw/pitch**, `glm::lookAt` e movimentação relativa à orientação. A lógica em `main.cpp` limita a movimentação no interior da casa e calcula colisões no plano XZ por caixas e círculos, subdividindo o deslocamento para evitar atravessar objetos com facilidade.

O sistema de interação utiliza:

- `ItemId` e `items[]`: identificadores e posições dos objetos.
- `aimedItem()`: escolhe um objeto próximo e alinhado à câmera.
- `promptFor()`: define o texto mostrado ao jogador.
- `interact()`: executa a ação ao pressionar `E`.
- `candleLit`, `candyGot`, `tombInspected`, `ghostFound` e demais estados globais: registram o progresso.

### Fluxo de renderização

Em cada quadro, o programa realiza aproximadamente as seguintes etapas:

1. Atualiza tempo, entrada, animações, estado da missão e colisões.
2. Configura câmera, projeção, materiais, luzes e parâmetros dos shaders.
3. Desenha os objetos opacos do interior, exterior, personagens e cenário.
4. Renderiza as bolas de cristal no **passe transparente**, com blending.
5. Desenha a neblina rasteira e os feixes de luar, também com blending.
6. Renderiza o **HUD por cima da cena**, com objetivos, tempo, mensagens e mira.
7. Troca os buffers e processa os eventos do GLFW.

### Interface

O HUD é desenhado com `stb_easy_font` **sem biblioteca externa de fontes**. Há um cronômetro no canto superior direito, objetivos no canto superior esquerdo, uma mira no centro, prompts contextuais e mensagens temporárias. Ao terminar, aparece o painel de vitória.

## Problemas comuns

### `Nao consegui carregar textura: assets/textures/...`

O arquivo não está no diretório esperado, tem nome ou extensão diferente, não foi copiado para a pasta do executável ou não pôde ser decodificado.

1. Confira nome e **extensão reais** (`.png`, `.jpg`, `.jpeg`).
2. Veja se o arquivo também existe em `build/Release/assets/textures/`, `build/Debug/assets/textures/` ou `build/assets/textures/`, conforme seu gerador CMake.
3. Execute o build novamente para repetir a cópia dos recursos.
4. Se uma imagem não carregar mesmo estando no lugar correto, tente salvá-la de novo como JPEG ou PNG em um editor.

Quando uma textura comum falha, `Texture.h` usa uma **textura branca de fallback**, permitindo que o jogo continue, mas o objeto pode perder o aspecto desejado. Quando um mapa de normais não carrega, o programa segue sem aquele relevo adicional.

### `Nao consegui abrir: shaders/...`

Provavelmente o executável foi iniciado com o **diretório de trabalho incorreto**. Execute dentro da pasta que contém o binário e as subpastas `shaders/` e `assets/`.

```powershell
cd build/Release
./sala.exe
```

Se estiver executando pelo VS Code ou outra IDE, ajuste o **working directory / cwd** para a pasta do executável.

### Erro do CMake durante o `FetchContent`

Verifique a conexão de internet, a instalação do Git, o acesso ao GitHub e as ferramentas de compilação. Confira também se a versão do CMake é **3.16 ou superior**.

### O jogo não abre ou a janela fica vazia

Verifique se a GPU e os drivers têm suporte a **OpenGL 3.3 Core Profile**. Examine as mensagens no terminal: `Shader.h` informa falhas ao abrir, compilar ou vincular os shaders, e a criação de janela também pode falhar caso o ambiente gráfico não seja compatível.

### O mouse desapareceu ao abrir o jogo

Esse comportamento é esperado: `GLFW_CURSOR_DISABLED` captura o cursor para a câmera FPS. Pressione **`Esc` para sair do jogo**.

### Textos do jogo sem acento

O `stb_easy_font` usado pelo HUD suporta caracteres **ASCII básicos**. Por isso, as mensagens dentro do jogo foram escritas sem acentos ou cedilha; isso não afeta este README.

### Como reiniciar a missão?

Feche o jogo com `Esc` e abra o executável novamente. **Não existe salvamento, carregamento de progresso ou opção de reiniciar a partida na versão atual.**

## Limitações e observações

- Jogo **local para um jogador**, controlado por teclado e mouse.
- A vitória é registrada ao **abrir a porta** com os requisitos completos; não há movimentação livre para além dos limites internos da casa.
- Não há áudio ou sistema de reprodução de música implementado no `main.cpp` analisado.
- O cristal usa transparência por blending, não vidro com refração fisicamente correta.
- A iluminação lunar através da janela usa uma aproximação geométrica; não há *shadow mapping* completo.
- Não há menu inicial, tela de configurações, sistema de salvamento ou pausa implementados.
- README baseado no código disponível em **`main`** e no `CMakeLists.txt`. Os comandos de build estão alinhados à configuração encontrada; **não foram executados e validados em cada sistema operacional durante a elaboração desta documentação**.

---

**Quebre a maldição, liberte os espíritos e encontre a saída. Feliz Halloween!** 🎃
