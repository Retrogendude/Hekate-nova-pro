# Hekate Nova Pro v33.1 beta

- Bloco Iniciar: ícone pequeno /icone/launch.png independente do fundo grande /icone/Launcher.png, substituível no Nova com pré-visualização correspondente.
- Créditos: HWTest-RCM 0.15 atribuído a Creased (Baptiste MOINE), em Software livre.
- Aspeto e opções: removidos os fundos das linhas Verificação das cópias e Efeitos de seleção; contorno de Opções do sistema uniformizado.
- Cópia e restauro emuMMC: contorno ativo no painel exterior, sem moldura dupla deslocada; comportamento dos botões preservado.
- Pacotes separados padrão e experimental de 8 GB para consolas fisicamente modificadas: Erista V1 e Mariko V2/Lite/OLED. Utiliza o sinalizador oficial de software Hekate, sem programar fusíveis.
- Diagnóstico RAM: mantém o modo de 8 GB ao iniciar e regressar, escolhe perfis por plataforma e capacidade e para se a capacidade mudar após o treino.
- Endereços altos com cálculos de 64 bits e relatório da capacidade real: 8176 MiB testáveis de 8192 MiB; 16 MiB protegidos para dados de trabalho.
- Atmosphère 1.11.0 ou posterior para modos acima de 4 GB: memmode=1 numa entrada Hekate fss0 ou enable_mem_mode=1 em exosphere.ini para fusee. As configurações existentes não são alteradas automaticamente.

Beta: compilações ARM e verificações PC concluídas. O modo de 8 GB, sobretudo o perfil Erista adaptado, exige validação em cada plataforma; não se certificam todos os chips de substituição. Relatórios chineses longos precisam de revisão linguística.

## v33

- Chinês simplificado como oitavo idioma nas definições e na configuração inicial.
- Fonte dedicada obrigatória apenas em chinês. A fonte pessoal é guardada para os restantes idiomas e o tamanho continua ajustável.
- Lecture.nyxf é a fonte automática dos idiomas latinos; o russo usa a família integrada. A fonte original do Hekate está disponível no seletor.
- Inclui três fundos estáticos, cinco ícones Nova e a imagem Launcher fornecida para o mosaico principal.
- Nova instalação: francês, Carbon Cyan, opacidade de 75% em janelas e ícones, texto de 30 px, cores pedidas e logótipo inicial claro. Mantém-se o ajuste original do relógio.
- Acesso visível a Auto Boot, atraso, Auto NoGC e Auto HOS Power Off. O brilho abre o controlo existente, sem duplicar o cursor.
- Seletor Auto Boot por páginas, no estilo Nova, sem lista vertical e com os índices originais do Hekate.
- Teste de memória separado com interface chinesa e caracteres de 16 × 16; compatível com os sete pacotes anteriores.
- Se a fonte chinesa estiver ausente ou inválida, é usado inglês nesta sessão. A fonte pessoal e as definições guardadas não são substituídas.
