#![allow(dead_code)]
#![allow(unused_variables)]

use std::{
    os::raw::{
        c_void,
        c_char,
    },
};

/// errors when building an architecture
pub enum ArchError {
    TooManyOperations,
    DuplicateMnemonic,
    InstructionArgsMismatch,
}

pub trait Operation {
    // takes the vm, executes code and
    // returns the next ip
    fn call(&mut self, mem : &mut Mem, ip: usize) -> Result<usize, ReturnCode>;
}

#[derive(Default)]
pub struct Cpu {
    ops : Vec<Box<dyn Operation>>,
}

impl Cpu {
    pub fn new_op(&mut self, opfn: Box<dyn Operation>) -> Result<u8, ArchError> {
        if self.ops.len() >= 256 {
            return Err(ArchError::TooManyOperations)
        }

        self.ops.push(opfn);
        Ok((self.ops.len() - 1) as u8)
    }
}

struct Instruction {
    /// mnemonic for instruction
    mnem: String,

    /// a list of the size of the instructions arguments
    /// (data after the op code)
    argn: Vec<u8>,

    /// the operation to perform
    op: Box<dyn Operation>,
}

pub struct InstructionSet {
    instrs : Vec<Instruction>,
}

impl InstructionSet {
    fn build(self) -> Result<(Compiler, Cpu), ArchError> {
        let mut cpu : Cpu = Default::default();
        let mut comp : Compiler = Default::default();

        for instr in self.instrs {
            let Instruction{mnem, argn, op} = instr;
            let op_code =  cpu.new_op(op)?;
            comp.new_instr(mnem, argn, op_code)?;
        }

        Ok((comp, cpu))
    }
}

pub struct CompilerInstruction {
    mnem: String,
    argn: Vec<u8>,
    op_code : u8,
}

impl CompilerInstruction {
    fn encode(&self, dst : &mut Vec<u8>, args : Vec<i32>) -> Result<(), ArchError> {
        dst.push(self.op_code);
        if self.argn.len() != args.len() {
            return Err(ArchError::InstructionArgsMismatch)
        }

        for i in 0..args.len() {
            let bytes = self.argn[i];
            let arg = args[i];


            if bytes >= 1 {
                dst.push(arg as u8);
            }
            if bytes >= 2 {
                dst.push((arg >> 8) as u8);
            }
            if bytes >= 3 {
                dst.push((arg >> 16) as u8);
            }
            if bytes >= 4 {
                dst.push((arg >> 24) as u8);
            }
        }

        Ok(())
    }
}

#[derive(Default)]
pub struct Compiler {
    instrs : Vec<CompilerInstruction>
}

impl Compiler {
    fn new_instr(&mut self, mnem: String, argn : Vec<u8>, op_code: u8) ->
        Result<(), ArchError> {

        self.instrs.push(CompilerInstruction{
            mnem: mnem,
            argn: argn,
            op_code: op_code,
        });

        Ok(())
    }

    fn get_instr(&self, mnem: String) -> Option<&CompilerInstruction> {
        for instr in &self.instrs {
            if instr.mnem == mnem {
                return Some(instr)
            }
        }

        None
    }

    fn compile(&self, s : String) -> Vec<u8> {
        vec![]
    }
}


mod default_ops {
    use super::*;

    type BoxOp = Box<dyn Operation>;

    impl<T> Operation for T
    where T : FnMut(&mut Mem, usize) -> Result<usize, ReturnCode> {
        fn call(&mut self, mem : &mut Mem, ip: usize) -> Result<usize, ReturnCode> {
            self(mem, ip)
        }
    }

    pub fn exit() -> BoxOp {
        Box::new(|_ : &mut Mem, _ | Ok(0))
    }

    pub fn jump_rel() -> BoxOp {
        Box::new(|mem : &mut Mem, ip| {
            let val = mem.get_text_u32(ip)? as i32;
            Ok((ip as i32 + val) as usize) // TODO improve this
        })
    }

    pub fn jump_abs() -> BoxOp {
        Box::new(|mem : &mut Mem, ip| {
            Ok(mem.get_text_u32(ip)? as usize)
        })
    }
    // TODO: jmp_mem?

    pub fn push_imm() -> BoxOp {
        Box::new(|mem : &mut Mem, ip| {
            let val = mem.get_text_u32(ip)? as i32;
            mem.push(val);
            Ok(ip+4)
        })
    }

    pub fn push_n() -> BoxOp {
        Box::new(|mem : &mut Mem, ip| {
            let val = mem.get_text_u8(ip)?;
            for _ in 0..val {
                mem.push(0);
            }

            Ok(ip+1)
        })
    }

    pub fn push_stack() -> BoxOp {
        Box::new(|mem : &mut Mem, ip| {
            let idx = mem.get_text_u8(ip)? as usize;

            mem.push(
                mem.get_stack_top(idx)? as i32);

            Ok(ip+1)
        })
    }

    pub fn push_mem() -> BoxOp {
        Box::new(|mem : &mut Mem, ip| {
            let idx = mem.get_text_u32(ip)? as usize;

            mem.push(
                mem.get_stack_bottom(idx)? as i32);

            Ok(ip+1)
        })
    }

    pub fn pop() -> BoxOp {
        Box::new(|mem : &mut Mem, ip| {
            mem.pop();
            Ok(ip)
        })
    }

    pub fn pop_n() -> BoxOp {
        Box::new(|mem : &mut Mem, ip| {
            let val = mem.get_text_u8(ip)?;
            for _ in 0..val {
                mem.pop();
            }

            Ok(ip+1)
        })
    }


    #[inline(always)]
    pub fn binary_op<F >(f : F) -> BoxOp
    where
        F : 'static + Fn(i32, i32) -> i32 {
        Box::new(move |mem : &mut Mem, ip| {
            let l = mem.get_stack_top(
                mem.get_text_u8(ip+1)? as usize)?;
            let r = mem.get_stack_top(
                mem.get_text_u8(ip+2)? as usize)?;
            let dst = mem.get_stack_top_mut(
                mem.get_text_u8(ip)? as usize)?;

            *dst = f(l, r);

            Ok(ip+3)
        })
    }

    #[inline(always)]
    pub fn binary_op_fail<F >(f : F) -> BoxOp
    where
        F : 'static + Fn(i32, i32) -> Result<i32, ReturnCode> {
        Box::new(move |mem : &mut Mem, ip| {
            let l = mem.get_stack_top(
                mem.get_text_u8(ip+1)? as usize)?;
            let r = mem.get_stack_top(
                mem.get_text_u8(ip+2)? as usize)?;
            let dst = mem.get_stack_top_mut(
                mem.get_text_u8(ip)? as usize)?;

            *dst = f(l, r)?;

            Ok(ip+3)
        })
    }

    pub fn add() -> BoxOp {
        binary_op(|l, r| l+r)
    }
    pub fn sub() -> BoxOp {
        binary_op(|l, r| l+r)
    }
    pub fn mul() -> BoxOp {
        binary_op(|l, r| l*r)
    }
    pub fn div() -> BoxOp {
        binary_op_fail(|l, r| {
            if l == 0 {
                Err(ReturnCode::DivideByZero)
            } else {
                Ok(l / r)
            }
        })
    }

    #[inline(always)]
    pub fn binary_op_f32<F >(f : F) -> BoxOp
    where
        F : 'static + Fn(f32, f32) -> f32 {
        binary_op(move |l,r|  {
            let ll = f32::from_bits(l as u32);
            let rr = f32::from_bits(r as u32);

            f(ll, rr).to_bits() as i32
        })
    }

    #[inline(always)]
    pub fn binary_op_f32_fail<F>(f: F) -> BoxOp
    where
        F : 'static + Fn(f32, f32) -> Result<f32, ReturnCode>{
        binary_op_fail(move |l,r|  {
            let ll = f32::from_bits(l as u32);
            let rr = f32::from_bits(r as u32);

            Ok(f(ll, rr)?.to_bits() as i32)
        })
    }

    pub fn fadd() -> BoxOp {
        binary_op_f32(|l, r| l+r)
    }
    pub fn fsub() -> BoxOp {
        binary_op_f32(|l, r| l+r)
    }
    pub fn fmul() -> BoxOp {
        binary_op_f32(|l, r| l*r)
    }
    pub fn fdiv() -> BoxOp {
        binary_op_f32_fail(|l, r| {
            if l == 0.0 {
                Err(ReturnCode::DivideByZero)
            } else {
                Ok(l / r)
            }
        })
    }

}


#[derive(PartialEq, Eq)]
pub enum ReturnCode {
    Failure, // generic error
    InvalidOpCode,
    DivideByZero,
    OutOfTextBounds,
    OutOfStackBounds,
}


#[derive(Default)]
pub struct Mem {
    stack : Vec<i32>,
    text : Vec<u8>,
}

impl Mem {
    #[inline(always)]
    fn get_op_code(&self, ip : usize) -> Result<u8, ReturnCode> {
        match self.text.get(ip) {
            None => Err(ReturnCode::OutOfTextBounds),
            Some(&op_code) => Ok(op_code),
        }
    }

    fn push(&mut self, val : i32) {
        self.stack.push(val);
    }

    fn pop(&mut self) {
        self.stack.pop();
    }

    fn get_stack_top(&self, idx : usize) -> Result<i32, ReturnCode> {
        let s = &self.stack;
        match s.get(s.len() - 1 - idx) {
            None => Err(ReturnCode::OutOfStackBounds),
            Some(&val) => Ok(val),
        }
    }

    fn get_stack_bottom(&self, idx : usize) -> Result<i32, ReturnCode> {
        let s = &self.stack;
        match s.get(idx) {
            None => Err(ReturnCode::OutOfStackBounds),
            Some(&val) => Ok(val),
        }
    }

    fn get_stack_top_mut(&mut self, idx : usize) -> Result<&mut i32, ReturnCode> {
        let l = self.stack.len();
        let s = &mut self.stack;
        match s.get_mut(l - 1 - idx) {
            None => Err(ReturnCode::OutOfStackBounds),
            Some(val) => Ok(val),
        }
    }

    pub fn get_text_u8(&self, ptr : usize) -> Result<u8, ReturnCode> {
        self.get_op_code(ptr)
    }

    #[allow(arithmetic_overflow)]
    pub fn get_text_u32(&self, ptr : usize) -> Result<u8, ReturnCode> {
        Ok(
            self.get_text_u8(ptr)? |
            self.get_text_u8(ptr+1)? << 8 |
            self.get_text_u8(ptr+2)? << 16 |
            self.get_text_u8(ptr+3)? << 24)
    }
}



pub fn step(mem: &mut Mem, cpu : &mut Cpu, mut ip : usize) -> Result<usize, ReturnCode> {
    let op_code = mem.get_op_code(ip)?;
    ip += 1;

    match cpu.ops.get_mut(op_code as usize) {
        None => Err(ReturnCode::InvalidOpCode),
        Some(opfn) => opfn.call(mem, ip),
    }
}

pub fn run(mem : &mut Mem, cpu: &mut Cpu) -> Result<(), ReturnCode> {
    let mut ip = 0;

    loop {
        ip = step(mem, cpu, ip)?;
        if ip == 0 {
            return Ok(())
        }
    }
}

// asteroids

const SHIP_CONTROL_FORWARD : u8 = 1;
const SHIP_CONTROL_LEFT : u8 = 2;
const SHIP_CONTROL_RIGHT : u8 = 4;
const SHIP_CONTROL_FIRE : u8 = 8;

#[repr(C)]
pub struct AsteroidRel {
    radius: f32,
    distance: f32,
    angle: f32,
    heading: f32,
}

pub struct ShipVM {
    ship_control : u8,
    n_asteroids: usize,
    ptr_asteroids: *mut c_void
}

impl Default for ShipVM {
    fn default() -> Self {
        Self {
            ship_control: 0,
            n_asteroids: 0,
            ptr_asteroids : std::ptr::null_mut(),
        }
    }
}

impl ShipVM {
    fn into_heap_ptr(self) -> *mut Self {
        let ret = Box::into_raw(Box::new(self));
        println!("alloced {:?}", ret);
        ret

    }

    unsafe fn free_heap_ptr(ptr : *mut Self) {
        assert!(!ptr.is_null());
        println!("dropping {:?}", ptr);

        std::mem::drop(Box::<Self>::from_raw(ptr));
    }

    fn ship_forward_on(&mut self) {
        self.ship_control |= SHIP_CONTROL_FORWARD
    }
    fn ship_left_on(&mut self) {
        self.ship_control |= SHIP_CONTROL_LEFT
    }
    fn ship_right_on(&mut self) {
        self.ship_control |= SHIP_CONTROL_RIGHT
    }

    fn ship_forward_off(&mut self) {
        self.ship_control &= !SHIP_CONTROL_FORWARD
    }
    fn ship_left_off(&mut self) {
        self.ship_control &= !SHIP_CONTROL_LEFT
    }
    fn ship_right_off(&mut self) {
        self.ship_control &= !SHIP_CONTROL_RIGHT
    }
}

// C interfaces

#[no_mangle]
pub extern "C" fn vm_c_compile(prog : *const c_char) -> *mut ShipVM {
    let ret : ShipVM = Default::default();

    ret.into_heap_ptr()
}

#[no_mangle]
pub extern "C" fn vm_c_free(vm : *mut ShipVM) {
    unsafe {
        ShipVM::free_heap_ptr(vm)
    }
}

#[no_mangle]
pub extern "C" fn vm_c_step(vm: *mut ShipVM,
                            ptr :*mut AsteroidRel,
                            len : usize) -> u8 {

    let asteroids = unsafe {
        std::slice::from_raw_parts(ptr, len)
    };

    SHIP_CONTROL_FORWARD | SHIP_CONTROL_LEFT | SHIP_CONTROL_FIRE
}


