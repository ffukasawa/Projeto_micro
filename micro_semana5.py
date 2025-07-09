import tkinter as tk
from tkinter import messagebox
from pymongo import MongoClient
from datetime import datetime, timedelta
import paho.mqtt.client as mqtt_client
import ssl
import certifi
from bson.json_util import dumps

# MongoDB
cliente = MongoClient("mongodb://localhost:27017/")
db = cliente["medicamentos"]
colecao_cadastro = db["registros"]
colecao_farmacia = db["farmacia"]


root = tk.Tk()
root.title("Página Inicial")
root.geometry("1200x700")
root.resizable(True, True)

dias_semana = ["Domingo", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado"]
textos_dias = []

frame_agenda = tk.Frame(root)
frame_agenda.pack(padx=10, pady=10)

for i, dia in enumerate(dias_semana):
    tk.Label(frame_agenda, text=dia, font=("Arial", 12, "bold")).grid(row=0, column=i)
    lista = tk.Listbox(frame_agenda, width=25, height=15, exportselection=False)
    lista.grid(row=1, column=i, padx=5)
    textos_dias.append(lista)
frame_estoque =tk.Frame(root)
frame_estoque.pack(padx =10, pady =10)

label_estoque = tk.Label(frame_estoque, text= "Quantidades nos Compartimentos:")
label_estoque.pack()

lista_estoque = tk.Listbox(frame_estoque, width=50, height = 10)
lista_estoque.pack()

def atualiza_estoque():
   
    lista_estoque.delete(0, tk.END)        
    docs = colecao_cadastro.find()
    for doc in docs:
        linha = f"Compartimento {doc['compartimento']}: {doc['remedio']} - {doc['quantidade_compartimento']} unidades"
        lista_estoque.insert(tk.END, linha)
       
   
   
   
   
def atualizar_agenda():
    dados = list(colecao_cadastro.find())
    for lista in textos_dias:
        lista.delete(0, tk.END)

    for i, dia in enumerate(dias_semana):
        registros = []
        for r in dados:
            if dia in r["dias"]:
                for horario in r["horarios"]:
                    registros.append({
                        "horario": horario,
                        "remedio": r["remedio"]
                    })
        registros.sort(key=lambda x: x["horario"])
        for r in registros:
            linha = f"{r['horario']} - {r['remedio']} "
            textos_dias[i].insert(tk.END, linha)
       
           
def pega_dados_banco():
    docs_cad = list(colecao_cadastro.find({},{"_id": 0,"quantidade_compartimento": 0, "data": 0}))
    json_cad = dumps(docs_cad)
    mqtt.publish("cadastro_remedio", payload = json_cad, qos=0)
   


def abrir_janela_remedio(modo="cadastro", dados=None):
    janela = tk.Toplevel(root)
    janela.title("Cadastro de Remédio" if modo == "cadastro" else "Editar Remédio")
   
   

    def gerar_horarios():
        return [(datetime.strptime("00:00", "%H:%M") + timedelta(minutes=30 * i)).strftime("%H:%M") for i in range(48)]

    tk.Label(janela, text="Remédio").grid(row=0, column=0, sticky='w', padx=10)
    entry_remedio = tk.Entry(janela)
    entry_remedio.grid(row=0, column=1, padx=5, sticky='ew')
   
    def busca_sugestao(prefixo):
        doc = colecao_farmacia.find_one({"nome": {"$regex": f"^{prefixo}", "$options": "i"}})
        if doc and "nome" in doc:
            return doc["nome"]
        else:
            return None

    def autocompleta(event):
        texto = entry_remedio.get()
        if len(texto) < 1 or event.keysym in ("BackSpace", "Left", "Right"):
            return
       
        sugestao = busca_sugestao(texto)
        if sugestao and sugestao.lower().startswith(texto.lower()) and sugestao.lower() != texto.lower():
            entry_remedio.delete(0, tk.END)
            entry_remedio.insert(0, sugestao)
            entry_remedio.select_range(len(texto), tk.END)

    entry_remedio.bind("<KeyRelease>", autocompleta)




    tk.Label(janela, text="Dias").grid(row=2, column=0, sticky='w', padx=10)
    listbox_dias = tk.Listbox(janela, selectmode=tk.MULTIPLE, height=7, exportselection=False)
    for d in dias_semana:
        listbox_dias.insert(tk.END, d)
    listbox_dias.grid(row=2, column=1, padx=10)

    tk.Label(janela, text="Horários").grid(row=4, column=0, sticky='w', padx=10)
    listbox_horas = tk.Listbox(janela, selectmode=tk.MULTIPLE, height=7, exportselection=False)
    horarios_formatados = gerar_horarios()
    for h in horarios_formatados:
        listbox_horas.insert(tk.END, h)
    listbox_horas.grid(row=4, column=1, padx=10)

    tk.Label(janela, text="Compartimento").grid(row=8, column=0, sticky='w', padx=10)
    spin_comp = tk.Spinbox(janela, from_=1, to=5)
    spin_comp.grid(row=8, column=1, sticky='w')

    tk.Label(janela, text="Quantidade Compartimento").grid(row=10, column=0, sticky='w', padx=10)
    spin_qtd = tk.Spinbox(janela, from_=1, to=50)
    spin_qtd.grid(row=10, column=1, sticky='w')

    if modo == "edicao" and dados:
        entry_remedio.insert(0, dados["remedio"])
        for i, d in enumerate(dias_semana):
            if d in dados["dias"]:
                listbox_dias.selection_set(i)
        for i, h in enumerate(horarios_formatados):
            if h in dados["horarios"]:
                listbox_horas.selection_set(i)
        spin_comp.delete(0, tk.END)
        spin_comp.insert(0, dados["compartimento"])
        spin_qtd.delete(0, tk.END)
        spin_qtd.insert(0, dados["quantidade_compartimento"])

    def salvar():
        nome = entry_remedio.get().strip()
        dias = [listbox_dias.get(i) for i in listbox_dias.curselection()]
        horas = [listbox_horas.get(i) for i in listbox_horas.curselection()]
        compart = int(spin_comp.get())
        qtd = int(spin_qtd.get())

        if not nome or not dias or not horas:
            messagebox.showerror("Erro", "Preencha todos os campos obrigatórios.")
            return

        doc_existente = colecao_cadastro.find_one({"compartimento": compart})
        if doc_existente and (modo == "cadastro" or doc_existente["remedio"] != dados["remedio"]):
            messagebox.showerror("Erro", f"Compartimento {compart} já está ocupado.")
            return

        if modo == "cadastro":
            if colecao_cadastro.find_one({"remedio": nome}):
                messagebox.showwarning("Aviso", "Remédio já cadastrado.")
                return

            colecao_cadastro.insert_one({
                "remedio": nome,
                "dias": dias,
                "horarios": horas,
                "compartimento": compart,
                "quantidade_compartimento": qtd,
                "data": datetime.now()
            })
           
            pega_dados_banco()
           
        else:
            colecao_cadastro.update_one({"_id": dados["_id"]}, {"$set": {
                "remedio": nome,
                "dias": dias,
                "horarios": horas,
                "compartimento": compart,
                "quantidade_compartimento": qtd,
                "data": datetime.now()
            }})
            pega_dados_banco()
         

        messagebox.showinfo("Sucesso", "Dados salvos com sucesso.")
        janela.destroy()
        atualizar_agenda()
        atualiza_estoque()

    tk.Button(janela, text="Cadastrar" if modo == "cadastro" else "Salvar Alterações", command=salvar).grid(row=12, column=0, columnspan=2, pady=10)

def editar_remedio():
    for i, lista in enumerate(textos_dias):
        if lista.curselection():
            dia_index = i
            selecionado = lista.curselection()[0]
            break
    else:
        messagebox.showwarning("Aviso", "Selecione um remédio da agenda.")
        return

    linha = textos_dias[dia_index].get(selecionado)
    horario, resto = linha.split(" - ")
    nome = resto.split(" (")[0].strip()

    doc = colecao_cadastro.find_one({"remedio": nome, "dias": dias_semana[dia_index], "horarios": horario})
    if not doc:
        messagebox.showerror("Erro", "Remédio não encontrado.")
        return

    doc_cad = colecao_cadastro.find_one({"remedio": nome}) or {}
    doc["compartimento"] = doc_cad.get("compartimento", 1)
    doc["quantidade_compartimento"] = doc_cad.get("quantidade_compartimento", 1)

    abrir_janela_remedio("edicao", doc)

def remover_remedio():
    for i, lista in enumerate(textos_dias):
        if lista.curselection():
            dia_index = i
            selecionado = lista.curselection()[0]
            break
    else:
        messagebox.showwarning("Aviso", "Selecione um remédio da agenda.")
        return

    linha = textos_dias[dia_index].get(selecionado)
    horario, resto = linha.split(" - ")
    nome = resto.split(" (")[0].strip()

    doc = colecao_cadastro.find_one({"remedio": nome, "dias": dias_semana[dia_index], "horarios": horario})
    if not doc:
        messagebox.showerror("Erro", "Remédio não encontrado.")
        return

    if messagebox.askyesno("Confirmar Remoção", f"Deseja remover o remédio '{nome}'?"):        
        colecao_cadastro.delete_one({"_id": doc["_id"]})
       
        pega_dados_banco()
       
        messagebox.showinfo("Removido", "Remédio removido com sucesso.")
        atualizar_agenda()
        atualiza_estoque()

# MENU
menu_bar = tk.Menu(root)
menu_cadastro = tk.Menu(menu_bar, tearoff=0)
menu_cadastro.add_command(label="Novo Remédio", command=lambda: abrir_janela_remedio("cadastro"))
menu_bar.add_cascade(label="Cadastro", menu=menu_cadastro)
root.config(menu=menu_bar)

# BOTÕES PRINCIPAIS
btn_editar = tk.Button(root, text="Editar Remédio", command=editar_remedio)
btn_editar.pack(padx=10, pady=5)
btn_remover = tk.Button(root, text="Remover Remédio", command=remover_remedio)
btn_remover.pack(padx=10, pady=5)

#MQTT
def on_connect(mqtt, dados_usuario, flags, codigo):
    mqtt.subscribe("hora_remedio")

def on_message(cliente, dados_usuario, msg):
    texto_recebido = msg.payload.decode("UTF-8")
    print("topico " + msg.topic + ": " + texto_recebido)
   
    if msg.topic == "hora_remedio":
        remedio = texto_recebido[8:]
        print(remedio)
       
        doc_cad = colecao_cadastro.find_one({"remedio": remedio})
        if not doc_cad:
            print("Remédio não encontrado.")
            return
        qtd_atual = doc_cad.get("quantidade_compartimento",0)
        if qtd_atual <= 0:
            print("Compartimento vazio")
           
            return
       
        doc_cad = colecao_cadastro.find_one({"remedio": remedio})
        if not doc_cad:
            print("Remédio não encontrado.")
            return
        qtd_dose = 1
       
        nova_qtd = max(qtd_atual - qtd_dose,0)
       
       
        colecao_cadastro.update_one({"remedio": remedio},{"$set": {"quantidade_compartimento": nova_qtd }})
       
        if nova_qtd == 0:
            print("ATENÇÃO: Compartimento vazio!")
            mqtt.publish("hora_remedio", payload=f"Acabou o estoque do {remedio}", qos=0)
       
        atualiza_estoque()
   
   
mqtt = mqtt_client.Client()
mqtt.tls_set(certifi.where())
mqtt.username_pw_set(username = "aula", password = "zowmad-tavQez")

mqtt.on_connect = on_connect
mqtt.on_message = on_message

mqtt.connect("mqtt.janks.dev.br", port=8883, keepalive=10)


# Inicializa
atualizar_agenda()
atualiza_estoque()
mqtt.loop_start()
root.mainloop()
mqtt.loop_stop()
