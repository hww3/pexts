
int main() {
	float w,h;

	w=595.00;
	h=842.00;
	object pdf=PDF.pdf("/tmp/something.pdf");
	pdf->begin_page(w,h);        
	int font=pdf->findfont("Helvetica","host",0);
	pdf->setfont(font,12.0);
	pdf->moveto(20.0,h-30);
	pdf->show("asdasjdkalsjdkasjdasdgashda jhdgahsdg as");
	pdf->rect(10.0,10.0,w-20,h-20);
	pdf->stroke();	

	pdf->end_page();
	pdf->close();
}
