// ALU
void add(Memoria *memoria);

void sub(Memoria *memoria);

void and(Memoria *memoria);

void xor(Memoria *memoria);

void or(Memoria *memoria);

void not(Memoria *memoria);


// BRANCHES

void jmp(Memoria *memoria);

void jz(Memoria *memoria);

void jnz(Memoria *memoria);

void jnegativo(Memoria *memoria);

void jnn(Memoria *memoria);

// PILHA

void push(Memoria *memoria);

void pop(Memoria *memoria);

// SUB ROTINAS

void call(Memoria *memoria);

void ret(Memoria *memoria);

////////////////

void dump(Memoria *memoria);

