use::core::arch::asm;
use core::panic::PanicInfo;

use::anyhow::Result;

use::std::thread;
use::std::time::Duration;

//Gpio use//
use::rppal::gpio::Gpio;
use::rppal::system::DeviceInfo;
//gpio pin set//


const GPIO_LED: u8 = 23;

fn main () -> Result<()>{
    println!("Blink led on {}.", DeviceInfo::new()?.model());

    let mut pin = Gpio::new()?.get(GPIO_LED)?.into_output();
    // Blink the LED by setting the pin's logic level high for 500 ms.
    pin.set_high();
    thread::sleep(Duration::from_millis(500));
    pin.set_low();

    Ok(())
}
